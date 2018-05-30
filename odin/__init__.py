from pyramid.config import Configurator
from .util import custom_json_renderer


def main(global_config, **settings):
    """ This function returns a Pyramid WSGI application.
    """
    config = Configurator(settings=settings)
    config.include('.models')
    config.include('.routes')
    config.scan()
    config.add_renderer('json', custom_json_renderer())
    return config.make_wsgi_app()
