#pragma once
#ifndef ITEM_DESIRABILITY_H
#define ITEM_DESIRABILITY_H

#include <unordered_map>
#include <string>
#include <locale>
#include <algorithm>
#include <iosfwd>
#include "json.h"

class JsonArray;
class JsonObject;
class JsonOut;

class item_desirability
{
    private:
        std::unordered_map<std::string, char> map_interest;

        const std::string get_string( const item *it ) const;

        mutable bool loaded;

    public:
        item_desirability() : loaded( false ) {}

        char get( const item *it );
        void set( const std::string str, char val );
        void increment( const item *it );
        void decrement( const item *it );
        void remove( const item *it );

        void clear();

        void load();
        bool save();

        void serialize( JsonOut &jsout ) const;
        void deserialize( const JsonArray &ja );
};

item_desirability &get_item_desirability();

#endif
