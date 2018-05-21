"""_models.task

Task
------

This module includes the SQLAlchemy Model for Task objects
"""
from sqlalchemy import Column, String, TIMESTAMP, Integer, ForeignKey
from .meta import Base
from sqlalchemy.orm import relationship


class Task(Base):
    """Task

    Provides commands and stores results
    """
    command = Column(String, nullable=False)
    results = Column(String)
    created = Column(TIMESTAMP(timezone=True))
    completed = Column(TIMESTAMP(timezone=True))
    status_id = Column(Integer, ForeignKey('status.rowid'))
    status = relationship('Status')
    client_id = Column(Integer, ForeignKey('client.rowid'))
    client = relationship('Client', back_populates='tasks')
