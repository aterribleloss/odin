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
    def to_json(self):
        return {'tuid': self.tuid, 'command': self.command,
                'status': self.status.name, 'results': self.results,
                'client': self.client.cuid, 'created': self.created,
                'completed': self.completed}

    tuid = Column(String, nullable=False)
    command = Column(String)
    results = Column(String)
    created = Column(TIMESTAMP(timezone=True))
    completed = Column(TIMESTAMP(timezone=True))
    status_id = Column(Integer, ForeignKey('status.rowid'))
    status = relationship('Status')
    client_id = Column(Integer, ForeignKey('client.rowid'))
    client = relationship('Client', back_populates='tasks')
    file = relationship("File", uselist=False, back_populates="task")


