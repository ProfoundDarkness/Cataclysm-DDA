#include "item_desirability.h"

#include <cstddef>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "mtype.h"
#include "cata_utility.h"
#include "debug.h"
#include "filesystem.h"
#include "game.h"
#include "item.h"
#include "item_contents.h"
#include "item_factory.h"
#include "itype.h"
#include "iuse.h"
#include "iuse_actor.h"
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

const std::string item_desirability::get_string( const item *it ) const
{
    // Want to know the name of the monster type if it's a corpse, since some monster corpses might be of more value (ie dissection proficiency or butchery results)
    if( it->is_corpse() ) {
        return it->get_mtype()->id.str();
    }
    // If an item is a pocket and only has one type of item inside, we really care about the contents.
    item_contents contents = it->get_contents();
    if( contents.num_item_stacks() == 1 ) {
        const item &contents_item = contents.only_item();
        // bug? item::has_label(), when called from outside item class, always returning false, workaround for string check instead.
        if( contents_item.label( 1 ).compare( "none" ) ) {
            return contents_item.typeId().str();
        }
    }
    // Otherwise just looking for the object, this is the general case.
    return it->typeId().str();
}

void item_desirability::clear()
{
    loaded = false;
    map_interest.clear();
}

char item_desirability::get( const item *it )
{
    std::string const str = get_string( it );
    if( map_interest.find( str ) == map_interest.end() ) {
        return ' ';
    }
    return map_interest[str];
}

void item_desirability::set( const std::string str, char val )
{
    if( val < '1' ) {
        val = '9';
    }
    if( val > '9' ) {
        val = '1';
    }
    map_interest[str] = val;
}

void item_desirability::increment( const item *it )
{
    std::string const str = get_string( it );
    if( map_interest.find( str ) == map_interest.end() ) {
        map_interest[str] = '1' - 1;    //init
    }

    char val = map_interest[str];
    val++;
    set( str, val );
    if( it->is_transformable() ) {
        const use_function fn = it->type->use_methods.find( "transform" )->second;
        const iuse_transform *act = dynamic_cast<const iuse_transform *>( fn.get_actor_ptr() );
        set( act->target.str(), val );
    }
}

void item_desirability::decrement( const item *it )
{
    std::string const str = get_string( it );
    if( map_interest.find( str ) == map_interest.end() ) {
        map_interest[str] = '9' + 1;
    }

    char val = map_interest[str];
    val--;
    set( str, val );
    if( it->is_transformable() ) {
        const use_function fn = it->type->use_methods.find( "transform" )->second;
        const iuse_transform *act = dynamic_cast<const iuse_transform *>( fn.get_actor_ptr() );
        set( act->target.str(), val );
    }
}

void item_desirability::remove( const item *it )
{
    std::string const str = get_string( it );
    if( map_interest.find( str ) != map_interest.end() ) {
        map_interest.erase( str );
    }
    if( it->is_transformable() ) {
        const use_function fn = it->type->use_methods.find( "transform" )->second;
        const iuse_transform *act = dynamic_cast<const iuse_transform *>( fn.get_actor_ptr() );
        map_interest.erase( act->target.str() );
    }
}

bool item_desirability::save()
{
    cata_path sFile = PATH_INFO::player_base_save_path_path() + ".idr.json";

    //Character not saved yet?
    const std::string player_save = PATH_INFO::player_base_save_path() + ".sav";
    if( !file_exist( player_save ) ) {
        return true;
    }

    return write_to_file( sFile, [&]( std::ostream & fout ) {
        JsonOut jout( fout, true );
        serialize( jout );
    }, _( "itemdesirability configuration" ) );
}

void item_desirability::load()
{
    loaded = false;
    cata_path sFile = PATH_INFO::player_base_save_path_path() + ".idr.json";
    if( !read_from_file_optional_json( sFile, [&]( const JsonValue & jv ) {
    deserialize( jv );
    } ) ) {
        if( save() ) {
            remove_file( sFile.generic_u8string() );
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

void item_desirability::deserialize( const JsonArray &ja )
{
    map_interest.clear();

    for( JsonObject jo : ja ) {
        const std::string sItemName = jo.get_string( "item" );
        const char val = jo.get_int( "value" );

        map_interest[sItemName] = val;
    }
}
