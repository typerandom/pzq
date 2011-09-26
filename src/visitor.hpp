/*
 *  Copyright 2011 Mikko Koppanen <mikko@kuut.io>
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.                 
 */

#ifndef PZQ_VISITOR_HPP
# define PZQ_VISITOR_HPP

#include "pzq.hpp"
#include "store.hpp"
#include "socket.hpp"
#include "time.hpp"

using namespace kyotocabinet;

namespace pzq
{
    class visitor_t : public DB::Visitor
    {
    private:
        boost::shared_ptr<pzq::socket_t> m_socket;
		boost::shared_ptr<pzq::datastore_t> m_store;
        uuid_t m_uuid;

    public:
        visitor_t ()
        {
            uuid_generate (m_uuid);
        }

        void set_socket (boost::shared_ptr<pzq::socket_t> socket)
        {
            m_socket = socket;
        }

        void set_datastore (boost::shared_ptr<pzq::datastore_t> store)
        {
            m_store = store;
        }

        bool can_write ();

    private:
        const char *visit_full (const char *kbuf, size_t ksiz, const char *vbuf, size_t vsiz, size_t *sp);

    };

    class expiry_visitor_t : public DB::Visitor
    {
    private:
        uint64_t m_time;
        uint64_t m_timeout;
        uint64_t m_frequency;
        boost::shared_ptr<boost::thread> m_thread;
        boost::shared_ptr<pzq::datastore_t> m_store;

    public:
        expiry_visitor_t (boost::shared_ptr<pzq::datastore_t> store) : m_store (store)
        {}

        void start ()
        {
            m_thread = boost::shared_ptr<boost::thread> (
                            new boost::thread (
                                boost::bind (&expiry_visitor_t::run, this)
                            )
                       );
        }

        void set_frequency (uint64_t frequency)
        {
            m_frequency = frequency;
        }

        void set_ack_timeout (uint64_t timeout)
        {
            m_timeout = timeout;
        }

        void run ()
        {
            while (1)
            {
                m_time = microsecond_timestamp ();
                m_store.get ()->iterate_inflight (this);
                boost::this_thread::sleep (boost::posix_time::microseconds (m_frequency));
            }
        }

        const char *visit_full (const char *kbuf, size_t ksiz, const char *vbuf, size_t vsiz, size_t *sp);
    };
}

#endif