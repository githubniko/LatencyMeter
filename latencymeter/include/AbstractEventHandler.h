// вз¤то из https://habr.com/ru/post/424593/
// нужен дл¤ реализации событий


#pragma once

#define __ASSERT_USE_STDERR

#include <assert.h>
#include "List.hpp"

template<class ...TParams>
class AbstractEventHandler
{
public:
    virtual void call(TParams... params) = 0;
protected:
    AbstractEventHandler() {}
};

template<class ...TParams>
class TEvent
{
    using TEventHandler = AbstractEventHandler<TParams...>;
public:
    TEvent() :
        m_handlers()
    {
    }
    ~TEvent()
    {
        for (int i = 0; i < m_handlers.getSize(); ++i)
            delete m_handlers[i];

        m_handlers.clear();
    }
    void operator()(TParams... params)
    {
        for (int i = 0; i < m_handlers.getSize(); ++i)
            m_handlers[i]->call(params...);
    }
    // добавл¤ет обрабочик событи¤
    void operator+=(TEventHandler& eventHandler)
    {
        m_handlers.add(&eventHandler);
    }
private:
    List<TEventHandler*> m_handlers;
};

template<class TObject, class ...TParams>
class MethodEventHandler : public AbstractEventHandler<TParams...>
{
    using TMethod = void(TObject::*)(TParams...);
public:
    MethodEventHandler(TObject& object, TMethod method) :
        AbstractEventHandler<TParams...>(),
        m_object(object),
        m_method(method)
    {
        assert(m_method != nullptr);
    }
    virtual void call(TParams... params) override final
    {
        (m_object.*m_method)(params...);
    }
private:
    TObject& m_object;
    TMethod m_method;
};

template<class TObject, class ...TParams>
AbstractEventHandler<TParams...>& createMethodEventHandler(TObject& object, void(TObject::* method)(TParams...))
{
    return *new MethodEventHandler<TObject, TParams...>(object, method);
}

#define METHOD_HANDLER( Object, Method ) createMethodEventHandler( Object, &Method )
#define MY_METHOD_HANDLER( Method ) METHOD_HANDLER( *this, Method )