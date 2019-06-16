#include "game.h"
#include "avatar.h"
#include "player.h"
#include "item_memmark.h"
#include "output.h"
#include "debug.h"
#include "item_factory.h"
#include "catacharset.h"
#include "translations.h"
#include "cata_utility.h"
#include "path_info.h"
#include "filesystem.h"
#include "input.h"
#include "worldfactory.h"
#include "itype.h"
#include "string_input_popup.h"

#include <stdlib.h>
#include <sstream>
#include <string>
#include <locale>

/* Intention is to add an item memory marker to allow for the game to remember what we have and don't have at our base while out raiding.
   This was inspired by two different elements.
   1> Watching things like Rycon Roleplays where they make the wrong decision about an item while out and about (leaving something behind that they needed/sacrificing useful stuff to bring back an unneeded duplicate).
   2> Myself personally a bit of a packrat, this allows me to keep track of items I want more of vs items I don't want anymore of (kind of weening away from packrat on EVERYTHING).
      -For myself personally without this feature I find myself using external tools to keep track.  Why do that when I can have the game do it for me and allow me to play more of the game?

   The original intention was to use a bit of color but that prooved unwise.
   Instead opted to add a new character and seemed relatively easy to implement.

   TODO:
   - Proper implementation of translation (in the key configuration screen).
   - Proper handling of when there is restricted space for viewing.  It works just fine on large screens but the default smaller display the new symbol gets blocked. (requires me to play the game some more again)
   - Consider a different symbol set as well as possible reduction in the number of symbols (feedback from players desired)
*/

item_memmark &get_item_memmark()
{
    static item_memmark single_instance = item_memmark();
    return single_instance;
}

const std::string item_memmark::clean_string( const std::string &sItemName ) const
{
    static const std::string find_string = " (";
    std::string::size_type pos;
    pos = sItemName.find( find_string );
    std::string const out = sItemName.substr( 0, pos );
    return out;
}

void item_memmark::clear()
{
    loaded = false;
    map_itemmem.clear();
}

char item_memmark::get( const std::string &sItemName )
{
    std::string const cleaned = clean_string( sItemName );
    if( map_itemmem.find( cleaned ) == map_itemmem.end() ) {
        return ' ';
    }
    return map_itemmem[cleaned];
}

void item_memmark::increment( const std::string &sItemName )
{
    std::string const cleaned = clean_string( sItemName );
    if( map_itemmem.find( cleaned ) == map_itemmem.end() ) {
        map_itemmem[cleaned] = '1';
        map_itemmem[cleaned]--;
    }

    ++map_itemmem[cleaned];
    if( map_itemmem[cleaned] > '9' ) {
        map_itemmem[cleaned] = '1';
    }
}

void item_memmark::decriment( const std::string &sItemName )
{
    std::string const cleaned = clean_string( sItemName );
    if( map_itemmem.find( cleaned ) == map_itemmem.end() ) {
        map_itemmem[cleaned] = '9';
        map_itemmem[cleaned]++;
    }
    --map_itemmem[cleaned];
    if( map_itemmem[cleaned] < '1' ) {
        map_itemmem[cleaned] = '9';
    }
}

void item_memmark::remove( const std::string &sItemName )
{
    std::string const cleaned = clean_string( sItemName );
    if( map_itemmem.find( cleaned ) != map_itemmem.end() ) {
        map_itemmem.erase( cleaned );
    }
}

bool item_memmark::save()
{
    auto savefile = FILENAMES["itemmem"];

    savefile = world_generator->active_world->folder_path() + "/" + base64_encode(
                   g->u.name ) + ".idr.json";
    const std::string player_save = world_generator->active_world->folder_path() + "/" + base64_encode(
                                        g->u.name ) + ".sav";
    if( !file_exist( player_save ) ) {
        return true;
    }

    return write_to_file( savefile, [&]( std::ostream & fout ) {
        JsonOut jout( fout, true );
        serialize( jout );
    }, _( "item memory marks" ) );
}

void item_memmark::load()
{
    loaded = false;
    std::string sFile = FILENAMES["itemmem"];
    sFile = world_generator->active_world->folder_path() + "/" + base64_encode(
                g->u.name ) + "idr.json";
    if( !read_from_file_optional( sFile, *this ) )
        if( save() ) {
            remove_file( sFile );
        }
    loaded = true;
    printf( "Loaded!\n" );
}

void item_memmark::serialize( JsonOut &json ) const
{
    json.start_array();
    for( auto &elem : map_itemmem ) {
        json.start_object();

        json.member( "item", elem.first );
        json.member( "value", elem.second );

        json.end_object();
    }
    json.end_array();
}

void item_memmark::deserialize( JsonIn &jsin )
{
    map_itemmem.clear();

    jsin.start_array();
    while( !jsin.end_array() ) {
        JsonObject jo = jsin.get_object();

        const std::string sItemName = jo.get_string( "item" );
        const char val = jo.get_int( "value" );

        map_itemmem[sItemName] = val;
    }
    printf( "deserialized\n" );
}
