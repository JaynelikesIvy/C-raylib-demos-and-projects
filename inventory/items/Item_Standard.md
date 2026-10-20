# INTRODUCTION TO .GITM AND USING MY SHIT PARSER

## ITEM NAME AND ITEM PROPERTY

To use the parser, you need to write information as such 
`'Item_name':Item_value`
Item name MUST be between '', and not "", and the item value must be after a colon

NOTE: last line NEEDS to be empty (you can write whatever as long as its not a key that is used to parse data)

## DEFINING TYPE

Defining a type is needed to be able to properly have the item be parsed, currently only the type "gun exists'
Defining a type looks like this 
`'Item_type':type'

## DEFINING VALUES

Values are needed for the item to work properly, currently only 5 values exist in this build
Damage -> integer
Firerate -> integer (in milliseconds)
ReloadSpeed -> integer (in milliseconds)
maxAmmo -> integer (total ammo allowed in mag)
totalAmmo -> integer (total ammo, surplus thats reloaded into the mag)