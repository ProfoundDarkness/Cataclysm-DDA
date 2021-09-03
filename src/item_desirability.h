#pragma once
#ifndef ITEM_DESIRABILITY_H
#define ITEM_DESIRABILITY_H

#include <unordered_map>
#include <string>
#include <locale>
#include <algorithm>
#include <iosfwd>
#include "json.h"

class item_desirability : public JsonSerializer, public JsonDeserializer
{
    private:
        std::unordered_map<std::string, char> map_interest;

        const std::string clean_string(const std::string &sItemName) const;

        mutable bool loaded;

    public:
        item_desirability() : loaded( false ) {}

        char get( const std::string &sItemName );
        void increment( const std::string &sItemName );
        void decrement( const std::string &sItemName );
        void remove( const std::string &sItemName );

        void clear();

        void load();
        bool save();

        using JsonSerializer::serialize;
        void serialize( JsonOut &json ) const override;
        void deserialize( JsonIn &jsin ) override;
};

item_desirability &get_item_desirability();

#endif
