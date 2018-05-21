from sqlalchemy.ext.declarative import declarative_base, declared_attr
from sqlalchemy.schema import MetaData
from sqlalchemy import Column, Integer


class MetaBase(object):
    """_models.meta

    Augmentation of the SQLAlchemy Declarative Base providing standard
    attributes across the set of models in this project.

    See:
    http://docs.sqlalchemy.org/en/latest/orm/extensions/declarative/mixins.html#augmenting-the-base

    """
    @declared_attr
    def __tablename__(self):
        return self.__name__.lower()

    rowid = Column(Integer, primary_key=True, autoincrement=True)


# Recommended naming convention used by Alembic, as various different database
# providers will autogenerate vastly different names making migrations more
# difficult. See: http://alembic.zzzcomputing.com/en/latest/naming.html
NAMING_CONVENTION = {
    "ix": "ix_%(column_0_label)s",
    "uq": "uq_%(table_name)s_%(column_0_name)s",
    "ck": "ck_%(table_name)s_%(constraint_name)s",
    "fk": "fk_%(table_name)s_%(column_0_name)s_%(referred_table_name)s",
    "pk": "pk_%(table_name)s"
}

metadata = MetaData(naming_convention=NAMING_CONVENTION)
Base = declarative_base(cls=MetaBase, metadata=metadata)
