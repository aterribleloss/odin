"""_models.status

Status
------

This module includes the SQLAlchemy Model for Status objects
"""
from sqlalchemy import Boolean, Column, String
from .meta import Base


class Status(Base):
    """Status

    Provides structure for Status model to SQLAlchemy.
    """
    name = Column(String, nullable=False)
    pending = Column(Boolean, nullable=False)
    running = Column(Boolean, nullable=False)
    done = Column(Boolean, nullable=False)

    @staticmethod
    def generate_statuses():
        """
        Generates a list of standard Status objects

        :return: list of Status
        """
        return [
            Status(
                name='Pending',
                pending=True,
                running=False,
                done=False
            ),
            Status(
                name='Running',
                pending=False,
                running=True,
                done=False
            ),
            Status(
                name='Done',
                pending=False,
                running=False,
                done=True
            )
        ]