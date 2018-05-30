"""_models.address

Address
-------

This module includes the SQLAlchemy model for ip objects.
"""

from sqlalchemy import Column, String, TIMESTAMP, Integer, ForeignKey
from sqlalchemy.orm import relationship
from .meta import Base
import json


class Address(Base):
    """
    Address

    Provides storage and tracking of remote addresses. At first just remote IP,
    future goals is to resolve domain names, support other protocols, etc.

    TODO need to make this many-to-many for clients <-> addresses, normalize.
    """
    def to_json(self):
        return {'address': self.address, 'first_seen': self.first_seen,
                'last_seen': self.last_seen}

    address = Column(String, nullable=False)
    last_seen = Column(TIMESTAMP(timezone=True))
    first_seen = Column(TIMESTAMP(timezone=True))
    client_id = Column(Integer, ForeignKey('client.rowid'))
    client = relationship('Client', back_populates='addresses')
