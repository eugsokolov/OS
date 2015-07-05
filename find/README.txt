Eugene Sokolov
Nov 7. 2013
Recreating of the Unix function 'find', outputting in a 'ls -l' fashion style.

This program will traverse through a given directory and output all files and folders
Arguments:
	-u user		Only list nodes owned by specified user
	-m mtime	Only list nodes which have not been modified in at least that many seconds
	-x		When specified, stay within the same volume which traversal began
	-l target	When specified print out information about nodes which are symlinks whose target resolves to the specified target

Usage:
1. make
2. Run ./search /tmp
