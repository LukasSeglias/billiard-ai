// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PERSISTENT_H__
#define __PERSISTENT_H__

#include <PvConfigurationReader.h>
#include <PvConfigurationWriter.h>

#include <list>


class IChanged
{
public:

    virtual bool HasChanged() = 0;
    virtual void ResetChanged() = 0;

};

typedef std::list<IChanged *> ChangedList;


class Persistent : public IChanged
{
public:

    Persistent();
    virtual ~Persistent() {}

    virtual PvResult Save( PvConfigurationWriter *aWriter ) = 0;
    virtual PvResult Load( PvConfigurationReader *aReader ) = 0;

    bool HasChanged();
    void ResetChanged();

private:

    PvString mBaseline;

};


#endif // __PERSISTENT_H__

