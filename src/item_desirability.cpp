#include "item_desirability.h"

#include <cstddef>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "cata_utility.h"
#include "debug.h"
#include "filesystem.h"
#include "game.h"
#include "item_factory.h"
#include "itype.h"
#include "json.h"
#include "output.h"
#include "path_info.h"

/* Intention is to add a new desirability property to advanced inventory.
 * The original idea was to add some color to the items like how autopickup-ed items have a mark of color on them (magenta).
 * Turns out adding a splash of color is actually kind of difficult since there are a variety of potential marks desired and I would
 * need a largely exhaustive list of colors that can happen which would also block colors from being used in the future...
 *
 * So falling back on the simpler idea, adding a symbol (number) to the start of the item.  This involves adding an additional column
 * to advanced inventory, the symbol can only ever be one character.
 */

item_desirability &get_item_desirability()
{
    static item_desirability single_instance = item_desirability();
    return single_instance;
}

const std::string item_desirability::clean_string( const item *it ) const
{
    std::string out;
    item_contents contents = it->get_contents();
    if( contents.num_item_stacks() == 1 ) {
        const item &contents_item = contents.only_item();
        out = contents_item.label( 1 );
    } else {
        out = it->label( 1 );
    }
    return out;
}

void item_desirability::clear()
{
    loaded = false;
    map_interest.clear();
}

char item_desirability::get( const item *it )
{
    std::string const cleaned = clean_string( it );
    if ( map_interest.find( cleaned ) == map_interest.end() )
        return ' ';
    return map_interest[cleaned];
}

void item_desirability::increment( const item *it )
{
    std::string const cleaned = clean_string( it );
    if ( map_interest.find( cleaned ) == map_interest.end() )
    {
        map_interest[cleaned] = '1';
        map_interest[cleaned]--;
    }

    ++map_interest[cleaned];
    if ( map_interest[cleaned] > '9' )
        map_interest[cleaned] = '1';
}

void item_desirability::decrement( const item *it )
{
    std::string const cleaned = clean_string( it );
    if ( map_interest.find( cleaned ) == map_interest.end() )
    {
        map_interest[cleaned] = '9';
        map_interest[cleaned]++;
    }
    --map_interest[cleaned];
    if ( map_interest[cleaned] < '1' )
        map_interest[cleaned] = '9';
}

void item_desirability::remove( const item *it )
{
    std::string const cleaned = clean_string( it );
    if ( map_interest.find( cleaned ) != map_interest.end() )
        map_interest.erase( cleaned );
}

bool item_desirability::save()
{
    std::string sFile = PATH_INFO::player_base_save_path() + ".idr.json";

    //Character not saved yet?
    const std::string player_save = PATH_INFO::player_base_save_path() + ".sav";
    if( !file_exist( player_save ) ) {
        return true;
    }

    return write_to_file( sFile, [&]( std::ostream &fout ) {
        JsonOut jout( fout, true );
        serialize( jout );
    }, _( "itemdesirability configuration" ) );
}

void item_desirability::load()
{
    loaded = false;
    std::string sFile = PATH_INFO::player_base_save_path() + ".idr.json";
    if( !read_from_file_optional_json( sFile, [&]( JsonIn & jsin ) {
        deserialize( jsin );
    }) ) {
        if (save()) {
            remove_file( sFile );
        }
    }
    loaded = true;
    printf( "Loaded!\n" );
}

void item_desirability::serialize( JsonOut &json ) const
{
    json.start_array();

    for( auto &elem : map_interest ) {
        json.start_object();

        json.member( "item", elem.first );
        json.member( "value", elem.second );

        json.end_object();
    }

    json.end_array();
}

void item_desirability::deserialize( JsonIn &jsin )
{
    map_interest.clear();

    jsin.start_array();
    while ( !jsin.end_array() ) {
        JsonObject jo = jsin.get_object();

        const std::string sItemName = jo.get_string( "item" );
        const char val = jo.get_int( "value" );

        map_interest[sItemName] = val;
    }
    printf( "deserialized\n" );
}
