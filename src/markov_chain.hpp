#ifndef SRC_MARKOV_CHAIN_HPP_
#define SRC_MARKOV_CHAIN_HPP_

#include <cstddef>
#include <iterator>
#include <set>
#include <random>

namespace porkfactor
{
    namespace conan
    {
        template<typename T, typename C = size_t>
        class markov_histogram;

        template<typename T>
        class markov_pdf;

        template<typename T>
        class markov_cdf;

        template<typename T>
        class markov_chain;

        template<typename T>
        class markov_state;

        template<typename T>
        class range
        {
        public:
            range(T const &center) : min_(center), max_(center) { };
            range(T const &min, T const &max) : min_(min), max_(max) { };

            T min() const { return min_; }
            T max() const { return max_; }

        private:
            T min_;
            T max_;
        };

        template<typename T>
        struct left_of_range : public std::binary_function<range<T>, range<T>, bool>
        {
            bool operator()(range<T> const &l, range<T> const &r)
            {
                return (l.min() < r.min()) && (l.max() <= r.max());
            }
        };

        template<typename T>
        struct left_of : public std::binary_function<T, T, bool>
        {
            bool operator()(T const &l, T const &r)
            {
                return (l <= r);
            }
        };

        class randomizer
        {
        public:
            double roll() { return static_cast<double>(dis_(gen_)); };

            static randomizer &instance() { return instance_; };

        private:
            randomizer() :
                gen_(rd_()),
                dis_(0.0f, 1.0f)
            {

            }

            static randomizer instance_;
            std::random_device rd_;
            std::mt19937 gen_;
            std::uniform_real_distribution<> dis_;
        };

        randomizer randomizer::instance_;

        template<typename T, typename C>
        class markov_histogram
        {
        public:
            typedef C size_type;
            typedef std::map<T, size_type> histogram_type;
            typedef typename histogram_type::iterator iterator;
            typedef typename histogram_type::const_iterator const_iterator;

            iterator begin() { return elements_.begin(); };
            iterator end() { return elements_.end(); };
            const_iterator begin() const { return elements_.begin(); };
            const_iterator end() const { return elements_.end(); };
            const_iterator cbegin() const { return elements_.cbegin(); };
            const_iterator cend() const { return elements_.cend(); };

            typename histogram_type::size_type size() const { return elements_.size(); };
            size_type total() const { return total_; };

            void add(T const &element, size_type count = 1)
            {
                elements_[element] += count;
                total_ += count;
            }

        private:
            histogram_type elements_;
            size_t total_;
        };

        template<typename T>
        class markov_pdf
        {
        public:
            typedef std::map<T, double> pdf_type;
            typedef typename pdf_type::iterator iterator;
            typedef typename pdf_type::const_iterator const_iterator;

            markov_pdf() { };

            markov_pdf(markov_histogram<T> const &histogram)
            {
                for(auto i : histogram)
                {
                    probabilities_[i.first] = static_cast<double>(i.second) / static_cast<double>(histogram.total());
                }
            }

            iterator begin() { return probabilities_.begin(); }
            iterator end() { return probabilities_.end(); }
            const_iterator begin() const { return probabilities_.begin(); }
            const_iterator end() const { return probabilities_.end(); }
            const_iterator cbegin() const { return probabilities_.cbegin(); }
            const_iterator cend() const { return probabilities_.cend(); }

        private:
            std::map<T, double> probabilities_;
        };

        template<typename T>
        class markov_cdf
        {
        public:
            markov_cdf(markov_pdf<T> const &pdf)
            {
                double tally { 0.0f };

                for(auto i : pdf)
                {
                    cdf_[range<double>(tally, tally + i.second)] = i.first;
                    tally += i.second;
                }
            }

            T probable(double d)
            {
                return cdf_[d];
            }

        private:
            std::map<range<double>, T, left_of_range<double>> cdf_;
        };

        template<typename T>
        class markov_chain_iterator : public std::iterator<std::forward_iterator_tag, T, ptrdiff_t, T*, T&>
        {
        public:
            markov_chain_iterator(markov_chain<T> *chain, T const &state) : chain_(chain), state_(state) { };
            markov_chain_iterator(markov_chain_iterator<T> const &) = default;

            T const &operator *() const {
                return state_;
            }

            T const &operator ->() const {
                return state_;
            }

            operator markov_chain_iterator<T const>()
            {
                return markov_chain_iterator<T const>(state_);
            }

            template<typename T_>
            bool operator == (markov_chain_iterator<T_> const &cmp) const
            {
                return state_ == cmp.state;
            };

            template<typename T_>
            bool operator != (markov_chain_iterator<T_> const &cmp) const
            {
                return state_ != cmp.state_;
            }

            markov_chain_iterator<T> &operator++ (void)
            {
                state_ = chain_->next_state(state_);
                return *this;
            }

            markov_chain_iterator<T> &operator++ (int)
            {
                markov_chain_iterator<T> tmp(*this);

                state_ = chain_->next_state(state_);

                return tmp;
            }

        private:
            markov_chain<T> *chain_;
            T state_;
        };

        template<typename T>
        class markov_chain
        {
        public:
            typedef size_t size_type;
            typedef std::map<T, markov_pdf<T>> map_type;
            typedef markov_chain_iterator<T> iterator;
            typedef markov_chain_iterator<T const> const_iterator;

            iterator begin() { return iterator(this, first_state()); }
            iterator end() { return iterator(this, nstate_); }
            const_iterator begin() const { return const_iterator(this, first_state()); }
            const_iterator end() const { return const_iterator(this, nstate_); }
            const_iterator cbegin() { return const_iterator(this, first_state()); }
            const_iterator cend() { return const_iterator(this, nstate_); }

            markov_chain() { };

            void add_state(T state, markov_pdf<T> pdf)
            {
                states_[state] = pdf;
            }

            T first_state() const
            {
                typename map_type::const_iterator i = states_.begin();

                std::advance(i, static_cast<typename map_type::size_type>(randomizer::instance().roll() * states_.size()));

                return i->first;
            }

            T next_state(T const &state) const
            {
                typename map_type::const_iterator i = states_.find(state);

                if(i != states_.end())
                {
                    markov_cdf<T> cdf(i->second);

                    return cdf.probable(randomizer::instance().roll());
                }

                return T();
            }

            size_type size() const { return states_.size(); };

        private:
            map_type states_;
            T nstate_;
        };
    }
}

#endif
