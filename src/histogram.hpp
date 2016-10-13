#ifndef SRC_HISTOGRAM_HPP_
#define SRC_HISTOGRAM_HPP_

#include <map>

namespace porkfactor
{
    namespace conan
    {
        template<typename T, typename C = size_t>
        class histogram
        {
        public:
            typedef size_t size_type;
            typedef std::map<T, C> histogram_type;
            typedef histogram_type::iterator iterator;
            typedef histogram_type::const_iterator const_iterator;

            void add(T const &element, size_type count = 1)
            {
                data_[element] += count;
            }



        private:
            histogram_type data_;
            size_type count_;
        };
    }
}

#endif
