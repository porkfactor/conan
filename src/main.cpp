
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "ngram.hpp"
#include "markov_chain.hpp"
#include <twitcurl.h>

namespace pc = porkfactor::conan;

typedef pc::ngram<std::string> ngram_type;
typedef pc::markov_histogram<ngram_type> histogram_type;
typedef pc::markov_chain<ngram_type> markov_type;

std::ostream &operator << (std::ostream &os, std::vector<std::string> &v)
{
    for(auto s : v)
    {
        os << s << std::endl;
    }

    return os;
}

void tokenize_stream(std::vector<std::string> &tokens, std::istream &is)
{
    static std::string const delimiters { " \t\r\n" };

    while(is.good())
    {
        std::string s;

        std::getline(is, s);

        std::string::size_type start = s.find_first_not_of(delimiters, 0);
        std::string::size_type end = s.find_first_of(delimiters, start);

        while(start != std::string::npos)
        {
            tokens.push_back(s.substr(start, end - start));

            start = s.find_first_not_of(delimiters, end);
            end = s.find_first_of(delimiters, start);
        }
    }
}

void tokenize_file(std::vector<std::string> &tokens, std::string const &filename)
{
    std::ifstream f(filename);

    if(f.is_open())
    {
        tokenize_stream(tokens, f);
    }
}

void munge_ngrams(std::map<ngram_type, histogram_type> &ngrams, std::vector<std::string> const &tokens)
{
    ngram_type last_ngram;
    auto end = tokens.cend();
    bool first { true };

    for(auto i = tokens.cbegin(); (i + 1) != end; i += 2)
    {
        ngram_type key { *i, *(i + 1) };

        if(first)
        {
            first = false;
        }
        else
        {
            ngrams[last_ngram].add(key);
        }

        last_ngram = key;
    }
}

void tweetshit(std::string const &s)
{
    std::string reply;
    twitCurl twat;

    twat.setTwitterUsername("");
    twat.setTwitterPassword("");
    twat.getOAuth().setConsumerKey(std::string(""));
    twat.getOAuth().setConsumerSecret(std::string(""));

    /* Step 1: Check if we alredy have OAuth access token from a previous run */
    std::string myOAuthAccessTokenKey("");
    std::string myOAuthAccessTokenSecret("");
    std::ifstream oAuthTokenKeyIn;
    std::ifstream oAuthTokenSecretIn;

    oAuthTokenKeyIn.open( "twitterClient_token_key.txt" );
    oAuthTokenSecretIn.open( "twitterClient_token_secret.txt" );

    char tmpBuf[1024];

    memset( tmpBuf, 0, sizeof(tmpBuf) );
    oAuthTokenKeyIn >> tmpBuf;
    myOAuthAccessTokenKey = tmpBuf;

    memset( tmpBuf, 0, sizeof(tmpBuf) );
    oAuthTokenSecretIn >> tmpBuf;
    myOAuthAccessTokenSecret = tmpBuf;

    oAuthTokenKeyIn.close();
    oAuthTokenSecretIn.close();

    if( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
    {
        /* If we already have these keys, then no need to go through auth again */
        printf( "\nUsing:\nKey: %s\nSecret: %s\n\n", myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str() );

        twat.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
        twat.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );
    }
    else
    {
        /* Step 2: Get request token key and secret */
        std::string authUrl;
        twat.oAuthRequestToken( authUrl );

        /* Else, pass auth url to twitCurl and get it via twitCurl PIN handling */
        twat.oAuthHandlePIN( authUrl );

        /* Step 4: Exchange request token with access token */
        twat.oAuthAccessToken();

        /* Step 5: Now, save this access token key and secret for future use without PIN */
        twat.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
        twat.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

        /* Step 6: Save these keys in a file or wherever */
        std::ofstream oAuthTokenKeyOut;
        std::ofstream oAuthTokenSecretOut;

        oAuthTokenKeyOut.open( "twitterClient_token_key.txt" );
        oAuthTokenSecretOut.open( "twitterClient_token_secret.txt" );

        oAuthTokenKeyOut.clear();
        oAuthTokenSecretOut.clear();

        oAuthTokenKeyOut << myOAuthAccessTokenKey.c_str();
        oAuthTokenSecretOut << myOAuthAccessTokenSecret.c_str();

        oAuthTokenKeyOut.close();
        oAuthTokenSecretOut.close();
    }

    if(twat.accountVerifyCredGet())
    {
        twat.getLastWebResponse(reply);
        std::cout << "twitCurl::accountVerifyCredGet web response:" << std::endl << reply << std::endl;

        if(twat.statusUpdate(s))
        {
            twat.getLastWebResponse(reply);
            std::cout << "twitCurl::statusUpdate web response:" << reply << std::endl;
        }
        else
        {
            twat.getLastCurlError(reply);
            std::cout << "twitCurl::statusUpdate error:" << reply << std::endl;
        }
    }
    else
    {
        twat.getLastCurlError(reply);
        std::cout << "twitCurl::accountVerifyCredGet() failed : " << reply << std::endl;
    }
}

int main(int argc, char **argv)
{
    int rv { -1 };
    std::vector<std::string> tokens;
    std::map<ngram_type, histogram_type> ngrams;
    markov_type markov_chain;

    for(auto i = 1; i < argc; i++)
    {
        tokenize_file(tokens, std::string(argv[i]));
    }

    std::cout << "found " << tokens.size() << " tokens" << std::endl;

    munge_ngrams(ngrams, tokens);

    std::cout << "found " << ngrams.size() << " ngrams" << std::endl;

    for(auto i : ngrams)
    {
        markov_chain.add_state(i.first, i.second);
    }

    std::cout << "found " << markov_chain.size() << " states" << std::endl;

    std::stringstream ss;
    for(auto i : markov_chain)
    {
        size_t length = i.get<0>().length() + i.get<1>().length() + 2;

        if((ss.str().length() + length) < 140)
        {
            if(ss.str().length() > 0)
            {
                ss << " ";
            }

            ss << i.get<0>() << " " << i.get<1>();
        }
    }

    std::cout << ss.str() << std::endl;

    if(false) tweetshit(ss.str());

    return rv;
}

