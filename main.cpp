#include "mime2.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>


int main(void)
{
    // slurp in the file
    std::ifstream t("sls.mime");
    std::stringstream buffer;
    buffer << t.rdbuf();

    MIME2::CONTENT c;
    if (c.Parse(buffer.str().c_str()) != MIME2::MIMEERR::OK)
        return 1;

    auto a1 = c.hval("Content-Type","boundary");
    if (a1.empty())
        return 1;

    std::vector<MIME2::CONTENT> Contents;
    MIME2::ParseMultipleContent2(buffer.str().c_str(), buffer.str().size(), a1.c_str(), Contents);

    // to get a specific one
    for (const auto &content : Contents)
    {
        if (content.hval("Content-Type").compare("application/mbms-envelope+xml") == 0)
        {
            std::cout << content.Content() << std::endl;
        }

    }
    std::cout << "==========================================================================" << std::endl;

    // Recreate it
    MIME2::CONTENTBUILDER cb;
    for (const auto &content : Contents)
    {
        MIME2::CONTENT e;
        e["Content-Type"] = content.hval("Content-Type").c_str();
        e["Content-Location"] = content.hval("Content-Location").c_str();
        e.SetData(content.Content().c_str());
        cb.Add(e);
    }

    MIME2::CONTENT cc;
    // Modified Build() for this example
    cb.BuildA(cc, "multipart/related", "application/mbms-envelope+xml");

    auto str = cc.SerializeToVector();
    std::cout << std::string(str.data(), str.size()) << std::endl;

    return 0;
}
