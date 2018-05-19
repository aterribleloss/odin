from cornice import Service
from ..tasks import add_thing
from random import randint

raven = Service(name='raven',
                path='/api/game/commander',
                description='Communicate with raven queues')


@raven.get()
def get_task(request):
    return {'raven': add_thing.delay(randint(0, 20), randint(0, 20))}


@raven.post()
def add_results(request):
    return {'raven': 'posted'}

