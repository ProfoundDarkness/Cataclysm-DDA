#pragma once
#ifndef ITEM_MEMMARK_H
#define ITEM_MEMMARK_H

#include <unordered_map>
#include <string>
#include <locale>
#include <algorithm>
#include <iosfwd>
#include "json.h"

class item_memmark : public JsonSerializer, public JsonDeserializer
{
    private:
        std::unordered_map<std::string, char> map_itemmem;

        const std::string clean_string( const std::string &sItemName ) const;

        mutable bool loaded;

    public:
        item_memmark() : loaded( false ) {}

        char get( const std::string &sItemName );
        void increment( const std::string &sItemName );
        void decriment( const std::string &sItemName );
        void remove( const std::string &sItemName );

        void clear();

        void load();
        bool save();

        using JsonSerializer::serialize;
        void serialize( JsonOut &json ) const override;
        void deserialize( JsonIn &jsin ) override;
};

item_memmark &get_item_memmark();

#endif
