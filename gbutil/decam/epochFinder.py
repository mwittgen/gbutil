''' Class for determining the calibration epoch that should be used for
an exposure taken at a given MJD.
'''
from astropy.time import Time
import numpy as np

def mjdOfEpoch(epoch):
    # Return mjd of date specified by 8-character epoch
    return Time(epoch[:4]+'-'+epoch[4:6]+'-'+epoch[6:8], 
                format='fits',scale='utc').mjd
class EpochFinder(object):
    '''Function class which returns the star flat epoch nearest to specifed input MJD
    that does not have an intervening camera event.  Returns '00000000' if no star flats
    occur in the same interval between events.
    '''
    # Epochs of star flats and of camera "events" when calibration changes.
    sfEpochs = ['20121223','20130221','20130829','20131115','20140118','20140807','20141105',
                '20150204','20150926', '20160209', '20160223','20160816','20161117','20170111',
                '20170214']
    warmups = ['20121230','20130512','20130722','20131015','20140512','20141201','20150625',
                '20150725','20150809','20150825','20160219','20161013','20161214','20161226']
    cooldowns=['20121226','20151126']
    nogood = '00000000'
    def __init__(self):
        # Place the epochs at ~midday Chile time of their stated date.
        self.sfMjds = np.array([mjdOfEpoch(e) for e in self.sfEpochs]) + 0.7
        self.eventMjds = np.array([mjdOfEpoch(e) for e in self.warmups + self.cooldowns]) + 0.7
        self.eventMjds.sort() 
        return
    def __call__(self, mjd):
        if mjd is None:
            return self.nogood
        # which events are before, after our mjd?
        before = mjd >= self.eventMjds
        # Mark which star flat MJDs are in same interval between events
        if not np.any(before):
            # Our mjd is before any events
            same = self.sfMjds < self.eventMjds[0]
        elif np.all(before):
            # Our mjd is after all events
            same = self.sfMjds >= self.eventMjds[-1]
        else:
            # Our mjd is between two events, get index of preceding one
            precede = np.where(before)[0][-1]
            same = np.logical_and(self.sfMjds>=self.eventMjds[precede],
                                  self.sfMjds <self.eventMjds[precede+1])
        
        if not same.any():
            # No star flats in the same event interval.
            return self.nogood
        sameIndices = np.where(same)[0]
        closest = np.argmin(np.abs(self.sfMjds[same]-mjd))
        return self.sfEpochs[sameIndices[closest]]