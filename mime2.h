#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <ctime>

namespace MIME2
{
    class CONTENT;

    enum class MIMEERR
    {
        OK = 0,
        INVALID = 1,
        NOTSIGNED = 2,
        ERRSIGN = 3,
    };

    inline void Split(const char*m, char del, std::vector<std::string>& result)
    {
        if (!m)
            return;
        std::stringstream ss(m);
        while (ss.good())
        {
            std::string substr;
            std::getline(ss, substr,del);
            result.push_back(substr);
        }
    }

    inline std::string& Trim(std::string& s,int j = 0)
    {
        while (s.length() && (j == 0 || j == 1))
        {
            if (s[s.length() - 1] == ' ' || s[s.length() - 1] == '\r' || s[s.length() - 1] == '\n' || s[s.length() - 1] == '\t')
                s.erase(s.end() - 1);
            else
                break;
        }
        while (s.length() && (j == 0 || j == 2))
        {
            if (s[0] == ' ' || s[0] == '\r' || s[0] == '\n' || s[0] == '\t')
                s.erase(s.begin());
            else
                break;
        }
        return s;
    }

    inline std::vector<char>& Trim(std::vector<char>& s, int j = 0)
    {
        while (s.size() && (j == 0 || j == 1))
        {
            if (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\r' || s[s.size() - 1] == '\n' || s[s.size() - 1] == '\t')
                s.erase(s.end() - 1);
            else
                break;
        }
        while (s.size() && (j == 0 || j == 2))
        {
            if (s[0] == ' ' || s[0] == '\r' || s[0] == '\n' || s[0] == '\t')
                s.erase(s.begin());
            else
                break;
        }
        return s;
    }

    inline std::vector<char>& TrimOnce(std::vector<char>& s)
    {
        if (s.size())
        {
            if (strncmp(s.data() + s.size() - 2, "\r\n", 2) == 0)
            {
                s.erase(s.end() - 1);
                s.erase(s.end() - 1);
            }
            else
                if (strncmp(s.data() + s.size() - 1, "\n", 1) == 0)
                {
                    s.erase(s.end() - 1);
                }
        }
        return s;
    }


    inline void Split(const char*m, const char* del, std::vector<std::string>& result)
    {
        if (!m || !del)
            return;
        size_t pos = 0;
        std::string token;
        std::string delimiter = del;
        std::string s = m;
        while ((pos = s.find(delimiter)) != std::string::npos)
        {
            token = s.substr(0, pos);
            result.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        result.push_back(s);
    }


    inline void BinarySplit(const char*m,size_t sz, const char* del, std::vector<std::vector<char>>& result)
    {
        if (!m || !del)
            return;
        size_t pos = 0;
        std::string token;
        std::string delimiter = del;
        std::string s;
        s.assign(m, sz);
        while ((pos = s.find(delimiter)) != std::string::npos)
        {
            token = s.substr(0, pos);
            std::vector<char> res;
            res.resize(token.size());
            memcpy(res.data(), token.data(), token.size());
            result.push_back(res);
            s.erase(0, pos + delimiter.length());
        }

        std::vector<char> res;
        res.resize(s.size());
        memcpy(res.data(), s.data(), s.size());
        result.push_back(res);
    }

    class HDRSTRING
    {
        std::vector<std::string> strs;

    public:

        std::vector<std::string>& getstrings() { return strs; }

        std::string Sub(const char* ga) const
        {
            if (!ga)
                return "";
            for (auto& a : strs)
            {
                const char* f1 = strchr(a.c_str(), '=');
                if (!f1)
                {
                    if (strcasecmp(a.c_str(), ga) == 0)
                        return a;
                    continue;
                }
                std::vector<char> leftpart(f1 - a.c_str() + 10);
                strncpy(leftpart.data(), a.c_str(), f1 - a.c_str());
                if (strncasecmp(leftpart.data(), ga,strlen(ga)) == 0)
                {
                    std::string r = f1 + 1;
                    if (r.length() && r[0] == '\"')
                        r.erase(r.begin());
                    if (r.length() && r[r.length() - 1] == '\"')
                        r.erase(r.end() - 1);
                    return r;
                }
            }
            return "";
        }

        MIMEERR Parse(const char* h)
        {
            strs.clear();
            Split(h, ';',strs);

            for (auto& a : strs)
            {
                Trim(a);
            }
            for (signed long long i = strs.size() - 1 ; i >= 0 ; i--)
            {
                if (strs[i].length() == 0)
                    strs.erase(strs.begin() + i);
            }

            return MIMEERR::OK;
        }

        std::string Serialize() const
        {
            std::string r;
            for (auto&s : strs)
            {
                if (r.length())
                    r += "; ";
                r += s;
            }
            return r;
        }

    };

    class HEADER
    {
        std::string left;
        HDRSTRING right;
        bool http = false;

    public:

        bool IsHTTP() const { return http; }
        std::string Left() const  { return left; }
        std::string Right() const { return right.Serialize(); }
        std::string Right(const char* sub) const { return right.Sub(sub); }
        HDRSTRING& rights() { return right; }

        std::vector<std::string> httpsplit()
        {
            std::vector<std::string> hd;
            Split(left.c_str(), ' ', hd);
            return hd;
        }


        void operator =(const char* l)
        {
            right.Parse(l);
        }

        MIMEERR Parse(const char* f,bool CanHTTP = false)
        {
            if (!f)
                return MIMEERR::INVALID;
            const char* a = strchr(f, ':');
            if (!a && !CanHTTP)
                return MIMEERR::INVALID;

            const char* a2 = strchr(f, ' ');
            if ((a2 < a) && CanHTTP)
                a = 0;

            if (!a && CanHTTP)
            {
                left = f;
                http = true;
                return MIMEERR::OK;
            }

            std::vector<char> d;
            d.resize(a - f + 10);
            //				strncpy_s(d.data(), d.size(), f, a - f);
            strncpy(d.data(), f, a - f);

            left = d.data();
            a++;
            while (*a == ' ')
                a++;
            right.Parse(a);

            return MIMEERR::OK;
        }

        std::string Serialize() const
        {
            if (http)
                return left;
            std::string r;
            r += left;
            r += ": ";
            r += right.Serialize();
            return r;
        }


    };

    class CONTENT
    {
    private:
        std::vector<HEADER> headers;
        std::vector<char> data;

    public:

        void clear()
        {
            headers.clear();
            data.clear();
        }

        std::vector<char> GetData() const
        {
            return data;
        }

        void SetData(std::vector<char>& x)
        {
            data = x;
        }

        void SetData(const char* a,ssize_t ss = -1)
        {
            if (ss == -1)
                ss = strlen(a);
            else
            {
                std::vector<char> d(ss);
                memcpy(d.data(), a, ss);
                SetData(d);
                return;
            }

            std::string j = a;
            Trim(j);
            if (j.empty())
                return;

            data.resize(j.length());
            memcpy(data.data(), j.c_str(), j.length());
        }

        std::string Content()  const
        {
            std::string a;
            auto d2 = data;
            d2.resize(d2.size() + 1);
            a = d2.data();
            d2.resize(d2.size() - 1);
            return a;
        }

        std::string hval(const char* left) const
        {
            for (auto& a : headers)
            {
                if (strcasecmp(a.Left().c_str(),left) == 0)
                    return a.Right();
            }
            return "";
        }

        std::string hval(const char* left,const char* rpart) const
        {
            for (auto& a : headers)
            {
                if (strcasecmp(a.Left().c_str(), left) == 0)
                    return a.Right(rpart);
            }
            return "";
        }

        HEADER& operator [](const char* l)
        {
            for (auto& h : headers)
            {
                if (strcasecmp(h.Left().c_str(), l) == 0)
                    return h;
            }
            HEADER h;
            std::string e = l;
            e += ": ";
            h.Parse(e.c_str());
            headers.push_back(h);
            return headers[headers.size() - 1];
        }

        MIMEERR Parse(const char* f, ssize_t ss = -1)
        {
            if (!f)
                return MIMEERR::INVALID;

            // Until \r\n\r\n
            const char* a2 = strstr(f, "\r\n\r\n");
            int jd = 4;
            const char* a21 = strstr(f, "\n\n");
            if (!a2 && !a21)
            {
                // No headers....
                SetData(f);
                return MIMEERR::OK;
            }
            if (a21 && !a2)
            {
                a2 = a21;
                jd = 2;
            }
            else if (!a21 && a2)
            {
                jd = 4;
            }
            else if (a21 < a2)
            {
                a2 = a21;
                jd = 2;
            }

            std::vector<char> hdrs;
            hdrs.resize(a2 - f + 10);
            strncpy(hdrs.data(),f, a2 - f);

            // Parse them
            std::vector<std::string> hd;
            Split(hdrs.data(), '\n', hd);
            for (auto& a : hd)
            {
                HEADER h;
                if ((a[0] == ' ' || a[0] == '\t') && headers.size())
                {
                    // Join with previous
                    auto& ph = headers[headers.size() - 1];
                    ph.rights().getstrings().push_back(Trim(a));
                    continue;
                }
                Trim(a);
                auto err = h.Parse(a.c_str());
                if (err != MIMEERR::OK)
                    return err;
                headers.push_back(h);
            }

            if (ss == -1)
                SetData(a2 + jd);
            else
                SetData(a2 + jd,ss - (a2 - f) - jd);
            return MIMEERR::OK;
        }


        std::vector<char> SerializeToVector() const
        {
            std::string r = SerializeHeaders();
            if (r.length())
                r += "\r\n";
            std::vector<char> x;
            x.resize(r.length());
            memcpy(x.data(), r.c_str(), r.length());
            auto os = x.size();
            x.resize(x.size() + data.size());
            memcpy(x.data() + os, data.data(),data.size());
            return x;
        }

        std::string SerializeHeaders() const
        {
            std::string r;
            for (auto& h : headers)
            {
                r += h.Serialize();
                r += "\r\n";
            }
            return r;
        }

    };


    uint32_t rand32()
    {
        srand(time(0));
        return ((rand() & 0x3) << 30) | ((rand() & 0x7fff) << 15) | (rand() & 0x7fff);
    }

    std::string gen_uuid4(void)
    {
        std::string uuid;
        char dst[38];
        snprintf(dst, 38, "%08x-%04x-%04x-%04x-%04x%08x",
            rand32(),                         // Generates a 32-bit Hex number
            rand32() & 0xffff,                // Generates a 16-bit Hex number
            ((rand32() & 0x0fff) | 0x4000),   // Generates a 16-bit Hex number of the form 4xxx (4 indicates the UUID version)
            (rand32() & 0x3fff) + 0x8000,     // Generates a 16-bit Hex number in the range [0x8000, 0xbfff]
            rand32() & 0xffff, rand32());     // Generates a 48-bit Hex number

        uuid = dst;
        return uuid;
    }


    class CONTENTBUILDER
    {
        std::vector<std::vector<char>> parts;
        std::string Boundary;

    public:

        CONTENTBUILDER()
        {
            std::string boundary = gen_uuid4();
            // convert to utf-8 here?
            Boundary = boundary;
        }

        void clear()
        {
            parts.clear();
        }

        void Add(char* Data)
        {
            std::vector<char> x(strlen(Data));
            memcpy(x.data(), Data, strlen(Data));
            parts.push_back(x);
        }

        void Add(CONTENT& c)
        {
            auto h1 = c.SerializeToVector();
            parts.push_back(h1);
        }

        std::vector<std::vector<char>>& GetParts() { return parts; }

        void Build(CONTENT& c,const char* Sign = 0)
        {
            c.clear();
            c["MIME-Version"] = "1.0";
            std::string a = "multipart/mixed";
            if (Sign)
                a = Sign;
            a += "; boundary=\"";
            a += Boundary;
            a += "\"";
            c["Content-Type"] = a.c_str();

            std::vector<char> d;
            for (auto& aa : parts)
            {
                std::string j = "--";
                j += Boundary;
                j += "\r\n";

                std::vector<char> jj(j.length() + aa.size() + 2);
                memcpy(jj.data(), j.c_str(), j.length());
                memcpy(jj.data() + j.length(), aa.data(), aa.size());
                memcpy(jj.data() + j.length() + aa.size(), "\r\n",2);
                auto es = d.size();
                d.resize(es + jj.size());
                memcpy(d.data() + es, jj.data(), jj.size());
            }

            auto es = d.size();
            d.resize(es + 2 + Boundary.size() + 4);
            memcpy(d.data() + es, "--",2);
            memcpy(d.data() + es + 2, Boundary.c_str(), Boundary.size());
            memcpy(d.data() + es + 2 + Boundary.size(), "--\r\n",4);

            c.SetData(d);
        }

        void BuildA(CONTENT& c,const char* Sign, const char* type = 0)
        {
            c.clear();
            std::string a = "multipart/mixed";
            if (Sign)
                a = Sign;
            a += ";\r\n";
            if (type)
            {
                a += " type=\"";
                a += type;
                a += "\";\r\n";
            }
            a += " boundary=\"";
            a += Boundary;
            a += "\"";
            c["Content-Type"] = a.c_str();

            std::vector<char> d;
            for (auto& aa : parts)
            {
                std::string j = "--";
                j += Boundary;
                j += "\r\n";

                std::vector<char> jj(j.length() + aa.size() + 2);
                memcpy(jj.data(), j.c_str(), j.length());
                memcpy(jj.data() + j.length(), aa.data(), aa.size());
                memcpy(jj.data() + j.length() + aa.size(), "\r\n",2);
                auto es = d.size();
                d.resize(es + jj.size());
                memcpy(d.data() + es, jj.data(), jj.size());
            }

            auto es = d.size();
            d.resize(es + 2 + Boundary.size() + 4);
            memcpy(d.data() + es, "--",2);
            memcpy(d.data() + es + 2, Boundary.c_str(), Boundary.size());
            memcpy(d.data() + es + 2 + Boundary.size(), "--\r\n",4);

            c.SetData(d);
        }


    };


	inline MIMEERR ParseMultipleContent2(const char* d, size_t sz, const char* del, std::vector<CONTENT>& Result)
    {
		if (!d || !del)
			return MIMEERR::INVALID;

		std::string dx = "--";
		dx += del;
		std::vector<std::vector<char>> r;

		BinarySplit(d, sz, dx.c_str(), r);

		if (r.size() < 2)
			return MIMEERR::INVALID;

		std::string delj = "--";
		delj += del;
		// First, check if [0] starts with it
		if (r[0].size() == 0 || strncmp(r[0].data(), delj.c_str(), delj.length()) != 0)
			r.erase(r.begin());

		// Check last if it starts with --
		if (strncmp(r[r.size() - 1].data(), "--", 2) == 0)
			r.erase(r.end() - 1);
		else
			return MIMEERR::INVALID;

		for (auto& a : r)
        {
			CONTENT c;
			Trim(a,2);
			TrimOnce(a);
			auto ra = a;
			ra.resize(ra.size() + 1);
			auto err = c.Parse(ra.data(),ra.size() - 1);
			if (err != MIMEERR::OK)
				return err;

			Result.push_back(c);
        }

		return MIMEERR::OK;
    }

};


