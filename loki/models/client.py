"""_models.client

Client
------

This module includes the SQLAlchemy Model for Client objects
"""

from sqlalchemy import Column, String
from .meta import Base
from sqlalchemy.orm import relationship
import json


class Client(Base):
    """Client

    Provides structure to store client details along with identifier and keys.
    """
    def to_json(self):
        return {'cuid': self.cuid, 'key': self.key,
                'shell_hint': self.shell_hint,
                'addresses': [a.to_json() for a in self.addresses],
                'tasks': [t.to_json() for t in self.tasks]}

    cuid = Column(String, unique=True)
    key = Column(String, unique=True)
    shell_hint = Column(String)  # temporary, tells us shell type
    addresses = relationship('Address', back_populates='client')
    tasks = relationship('Task', back_populates='client')
