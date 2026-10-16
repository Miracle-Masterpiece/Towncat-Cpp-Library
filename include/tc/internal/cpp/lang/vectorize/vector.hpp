#ifndef DD3DDB76_4A03_42C0_BF9F_1BCC7EBEEC43
#define DD3DDB76_4A03_42C0_BF9F_1BCC7EBEEC43

namespace tc
{

/**
 * @since 1.1.0
 */
template<typename Ttype, unsigned Tsize>
class vector {
    typedef vector<Ttype, Tsize> Tvector;
    Ttype m_data[Tsize];
public:
    
    vector() :
        m_data{} 
    {
    }

    template<
        typename... Args,
        typename = typename std::enable_if<sizeof...(Args) == Tsize>::type
    >
    vector(Args&&... args) :
        m_data{static_cast<Ttype>(args)...} 
    {
    }

    Tvector operator+ (const Tvector& rhs) const {
        Tvector result;
        for (unsigned i = 0; i < Tsize; ++i)
            result.m_data[i] = m_data[i] + rhs.m_data[i];
        return result;
    }
    
    Tvector operator- (const Tvector& rhs) const {
        Tvector result;
        for (unsigned i = 0; i < Tsize; ++i)
            result.m_data[i] = m_data[i] - rhs.m_data[i];
        return result;
    }
    
    Tvector operator* (const Tvector& rhs) const {
        Tvector result;
        for (unsigned i = 0; i < Tsize; ++i)
            result.m_data[i] = m_data[i] * rhs.m_data[i];
        return result;
    }
    
    Tvector operator/ (const Tvector& rhs) const {
        Tvector result;
        for (unsigned i = 0; i < Tsize; ++i)
            result.m_data[i] = m_data[i] / rhs.m_data[i];
        return result;
    }

    void to_array_unaligned(Ttype* arr) const {
        for (unsigned i = 0; i < Tsize; ++i)
            arr[i] = m_data[i];
    }
    
    void to_array_aligned(Ttype* arr) const {
        for (unsigned i = 0; i < Tsize; ++i)
            arr[i] = m_data[i];
    }
};

} //namespace tc

#endif /* DD3DDB76_4A03_42C0_BF9F_1BCC7EBEEC43 */
