
# ArcDPS Boon Table

![preview](https://i.imgur.com/K10miQG.png)

## Usage

Alt+Shift+B (overwrites Arc’s default boon table hotkey) or checkbox in the arc options

Right-click for options menu

## TODO:

    use disabled color for on not-hover
    show in/out of combat
    add profession column
    add optional profession colors
    add scholar uptime
    add seaweed salad uptime
    make columns same order as dps.report
    option to re-order columns
    show which column is used for sorting currently
    add line between subgroups when sorting by subgroup
    add condi generation/uptime
    add style options similar to Arc
    add option to ‘Hide Zero Buffs’
    look into self being 1st player in list
    make the subgroups bake use a tuple for (subgroup number,<players in subgroup>)
    Add signet uptime
    does the subgroup hiding work well?
    stances
    guardian signet share

## Changelog

    July 20 2019
        Add dynamic version string
        Make boon generation not cap at 100% for duration stacking boons
        Use arcdps exports instead of reading the arc ini manually
        Add tracking for Signet of inspiration (more coming soon)
        Remember boons on players bars when they exit combat instead of assuming they will expire before player reenters combat
            Fixes an accuracy bug mainly visible in FotM / WvW
        Significant performance improvements (30%-50% reduction in cpu usage to render the table)
        Misc code cleanup
    May 11 2019
        Remove Glyph of Empowerment tracking since it’s no longer in the game
        Add tracking for stealth and superspeed
    April 23 2019
        Fix a rare crash in cases of extreme boon generation
    February 1 2019
        Add initial boon generation support
            I’ve tested this a considerable amount myself, but please let me know of any oddities regarding generation. (Uptime is still as accurate as ever)
        Fix showing old subsquads when in a party
        Hide UI during loading screens/character select
    January 8 2019
        Fix crash when launching GW2 with the UI hidden
        Rework when the ‘Total’ row shows
    January 1 2019
        Completely rewrite the table’s UI code
            let me know of any issues and feedback
        Fix keyboard input requiring the mechanics log to also be running
        Fix subgroups not updating when a known player joins the 2nd time
        Fix non-bar display being off by 2 decimal places
        Change menus to mimic arcdps’s new style
    December 25 2018
        Add tracking for water spirit
        Add initial support for pre-booning (mainly impacts fractals)
        Code Cleanup
        Fix a bug where some boon applications would have vastly negative duration
    December 18 2018
        Add additional checks for empty player data
    December 13 2018
        Fix boon uptime calculation potentially overflowing in extended combat
        Add option to show uptime as a % instead of a progress bar
        Make options menu work anywhere on the window
        Make subgroup column only show if the subgroup section shows
        Add additional checks for empty player data
        Add support for boon extensions (requires new arcdps, not yet released)
    November 29 2018
        Add tracking for swiftness and resistance uptime
    November 7 2018
        Code cleanup
        Don’t show if a player currently has a boon as per Terms of Service (oops)
            Thanks to deltaconnected for pointing this out
    October 9 2018
        Simplify internal tracking code (breakage possible)
        Make a players initial subgroup accurate
    October 2 2018
        Update for arcdps cbtevent changes
    Sceptember 17 2018
        Fix some issues on player removes
    September 11 2018
        Fix potential crash when leaving a squad
        Fixed Total line showing after joining/leaving a squad
        Add optional “Y”/”N” for if a player has a boon currently
        Fix header column alignment
        Add rt-click button to clear the table as a temporary fix for arc’s missing remove notifies
        Update to match combat api changes
    August 18 2018
        Initial pre-release
