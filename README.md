# Loki

## Goals

Make a simple C2 / Implant Pair.  Prepare to extend to more advanced features.


# Implants

Make one in GraalVM (https://www.graalvm.org/)?

# Technologies

  - [Pyramid (WebApp Framework)](https://trypyramid.com/)
  - [Cornice (REST Framework)](https://cornice.readthedocs.io/en/latest/)
  - [SQLAlchemy (ORM)](http://www.sqlalchemy.org/)
  - [Pyramid_Celery (Queues)](https://github.com/sontek/pyramid_celery)
  

# Notes

## Querying
When in a request, the `request.dbsession` variable is your sqlalchemy session

```python
    try:
        query = request.dbsession.query(MyModel)
        one = query.filter(MyModel.name == 'one').first()
    except DBAPIError:
        return Response(db_err_msg, content_type='text/plain', status=500)
```

### Queueing
In the case of needing queueing these notes are helpful.  I'm changing from using a queue as primary mechanism to using
DB entries via models for tasking and such.

  - Run `rabbitmq-server` (feel free to run in the background with `rabbitmq-server -detatched`)
  - Configure RabbitMQ
    - $ rabbitmqctl add_user myuser mypassword
    - $ rabbitmqctl add_vhost myvhost
    - $ rabbitmqctl set_user_tags myuser mytag
    - $ rabbitmqctl set_permissions -p myvhost myuser ".*" ".*" ".*"
  - Never outright kill the server, just use `rabbitmqctl stop` to gracefully stop the server.
  - Management console at http://localhost:15672/#/  with guest:guest for local attachments
  - Fire up `celery worker -A loki.tasks -E --ini development.ini` to run the worker.

#### Development Notes
  
  - When calling a task, you must use `method.delay(params)` instead of the usual `method.params()`
  - Parameters sent to a task must be JSON serializable (see https://docs.python.org/3/library/json.html)
# Thoughts

  - Keep things obscure.  API can be something like `/api/game/implantcodename`
  - Don't just use an ORM and hope for the best. Secure and validate all endpoints.



## Pyramid Instructions

```
- Change directory into your newly created project.
    cd loki
- Create a Python virtual environment.
    python3 -m venv env
- Upgrade packaging tools.
    env/bin/pip install --upgrade pip setuptools
- Install the project in editable mode with its testing requirements.
    env/bin/pip install -e ".[testing]"
- Configure the database.
    env/bin/initialize_loki_db development.ini
- Run your project's tests.
    env/bin/pytest
- Run your project.
    env/bin/pserve development.ini
```