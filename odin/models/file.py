"""_models.file

File
------

This module includes the SQLAlchemy Model for File objects
"""
from sqlalchemy import Column, String, TIMESTAMP, Integer, ForeignKey
from .meta import Base
from sqlalchemy.orm import relationship


class File(Base):
    """File

    Provides commands and stores results
    """
    def to_json(self):
        return {'name': self.name, 'verb': self.verb,
                'type': self.type, 'location': self.location,
                'remote_path': self.remote_path, 'hash': self.hash}

    name = Column(String, nullable=False)
    verb = Column(String, nullable=False)  # put, get
    type = Column(String)  # TODO use python-magic to determine type
    location = Column(String)
    remote_path = Column(String)
    meta = Column(String)
    hash = Column(String)
    task_id = Column(Integer, ForeignKey('task.rowid'))
    task = relationship('Task', back_populates='file')
