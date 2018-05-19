from pyramid_celery import celery_app as app

import time
import random


@app.task(name='addition')
def add_thing(x, y):
    return x+y


def includeme(config):
    config.configure_celery('etc/development.ini')
