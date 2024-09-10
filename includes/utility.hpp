template<typename T, std::size_t N>
class FixedSizeVector
{
public:
    using Underlying = std::array<T, N>;
    using CUnderlying = std::array<const T, N>;

    void push_back(const T &obj)
    {
        (*m_end++) = obj;
    }

    Underlying::iterator begin()
    {
        return m_arr.begin();
    }

    const CUnderlying::iterator begin() const
    {
        return m_arr.cbegin();
    }

    Underlying::iterator end()
    {
        return m_end;
    }

    const CUnderlying::iterator end() const
    {
        return m_end;
    }

    const T &operator[](std::size_t idx) const
    {
        return m_arr[idx];
    }

    std::size_t size() const 
    {
        return std::distance(begin(), end());
    }

private:
    Underlying m_arr;
    Underlying::iterator m_end{m_arr.begin()};

};