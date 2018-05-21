"""_models.client

Client
------

This module includes the SQLAlchemy Model for Client objects
"""

from sqlalchemy import Column, String
from .meta import Base
from sqlalchemy.orm import relationship


class Client(Base):
    """Client

    Provides structure to store client details along with identifier and keys.
    """
    uid = Column(String, unique=True)
    key = Column(String, unique=True)
    addresses = relationship('Address', back_populates='client')
    tasks = relationship('Task', back_populates='client')
