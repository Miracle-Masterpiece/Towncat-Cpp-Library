#include <cpp/lang/io/properties.hpp>
#include <cpp/lang/utils/date.hpp>

namespace tc {

    properties::properties(tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_values(allocator) {

    }

    bool properties::is_empty() const {
        return m_values.is_empty();
    }

    void properties::set(const string& key, const string& value) {
        m_values.put(key, value);
    }

    void properties::set(const char* key, const char* value) {   
        m_values.put(string(key, m_allocator), string(value, m_allocator));
    }

    const string& properties::get(const string& key) const {
        return m_values.get(key);
    }

    const string& properties::get_or_default(const string& key, string& _default) const {
        return m_values.get_or_default(key, _default);
    }

    const string& properties::get(const char* key) const {
        return get(string(key, m_allocator));
    }

    void properties::save(ostream& out) const {
        const char NEW_LINE = '\n';
        const char ASSIGN   = '=';

        {//store date
            date now = date::now();
            string date = now.to_string(m_allocator);
            out.write(date.cstr(), date.length() + 1);
            out.write(&NEW_LINE, 1);
        }

        for (const pair<tc::string, tc::string>& e : m_values)
        {    
            const string& key     = e.first();
            const string& value   = e.second();
            
            out.write(key.cstr(), key.length());
         
            out.write(&ASSIGN, 1);                      //add "="
         
            out.write(value.cstr(), value.length());
         
            out.write(&NEW_LINE, 1);                    //add "\n"
        }
    }

    void properties::put_property(string& key, string& value) {
        key.trim();
        value.trim();

        if (!key.is_empty())
            m_values.put(key, value);

        key.clear();
        value.clear();
    }

    void properties::load(istream& in)
    {
        JSTD_DEBUG_CODE(
            if (m_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator must be != null");
        )

        enum struct state {KEY, VALUE, COMMENT};
        
        int reader;
        state state = state::KEY;

        string key (m_allocator);
        string val (m_allocator);

        while ((reader = in.read()) != -1)
        {
            if (state == state::KEY)
            {
                if (reader == '#')
                {
                    state = state::COMMENT;
                    continue;
                }
                if (reader == '=')
                {
                    state = state::VALUE;
                    continue;
                }
                key.append((char) reader);
            }
            else if (state == state::VALUE)
            {
                if (reader == '\n')
                {
                    put_property(key, val);
                    state = state::KEY;
                    continue;
                }
                val.append((char) reader);
            }
            else if (state == state::COMMENT)
            {
                if (reader == '\n')
                {
                    state = state::KEY;
                    continue;
                }
            }
        }
        
        put_property(key, val);
        
    }
}