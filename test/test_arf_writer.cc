#include <iostream>
#include <cassert>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/iostreams/stream.hpp>

#include "jill/data_source.hh"
#include "jill/data_writer.hh"
#include "jill/file/arf_writer.hh"


using namespace std;
using namespace jill;

struct redirector : public event_logger {
        redirector(event_logger &w) : _w(w), was_redirected(false) {
                w.redirect(*this);
        }
        ostream & log() { return _w.log(); }
        streamsize log(char const * msg, streamsize n) {
                was_redirected = true;
                return _w.log(msg, n);
        }
        event_logger & _w;
        bool was_redirected;
};

boost::shared_ptr<data_writer> writer;
boost::shared_ptr<data_source> client;

int
main(int, char**)
{
        map<string,string> attrs = boost::assign::map_list_of("experimenter","Dan Meliza")
                ("experiment","write stuff");


        writer.reset(new file::arf_writer("test_arf_writer","test.arf", attrs, client, 0));

        writer->log() << "a log message" << std::endl;
        assert(!writer->ready());

        void * buf = malloc(sizeof(period_info_t) + 1024 * sizeof(sample_t));
        period_info_t * period = reinterpret_cast<period_info_t*>(buf);
        period->time = 0;
        period->nframes = 1024;
        period->arg = 0;

        writer->new_entry(0);
        assert(writer->ready());

        for (int i = 0; i < 10; ++i) {
                for (int j = 0; j < 2; ++j ) {
                        nframes_t n = writer->write(period);
                        assert(n == period->nframes);
                }
                if (i > 0)
                        assert(writer->aligned());
                period->time += period->nframes;
        }

        writer->write(period);
        assert(!writer->aligned());

        writer->close_entry();
        assert(!writer->ready());

        redirector r(*writer);
        writer->log() << "another log message" << std::endl;
        assert(r.was_redirected);
}
