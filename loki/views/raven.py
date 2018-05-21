from cornice import Service
from ..tasks import add_thing
from random import randint
from ..models import Client
from secrets import token_hex
from pyramid.exceptions import HTTPNotFound
import json



raven_registration = Service(name='ravenreg',
                             path='/api/game/login',
                             description='Setup uid, key')

@raven_registration.post()
def get_login(request):
    # super secure SMTP keyword gets you keys to be commanded
    if 'ehlo' in request.POST.keys():
        client = Client(uid=token_hex(8), key=token_hex(16))
        request.dbsession.add(client)
        response = {}
        raven = {'uid': client.uid, 'key': client.key}
        response['raven'] = raven
        return response

    raise HTTPNotFound  # super secret if you don't have the password


raven_tasking = Service(name='raventask',
                        path='/api/game/commander',
                        description='Communicate with raven tasks')


@raven_tasking.get()
def get_task(request):
    return {'raven': add_thing.delay(randint(0, 20), randint(0, 20))}


@raven_tasking.post()
def add_results(request):
    return {'raven': 'posted'}

