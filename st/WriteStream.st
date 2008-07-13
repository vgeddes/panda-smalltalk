


"Accessing"

WriteStream method!
next
	self shouldNotImplement!

WriteStream method!
nextPut: anObject	
	position >= writeLimit
		ifTrue: [self pastEndPut: anObject]
		ifFalse: [ position := position + 1.
				   collection at: position put: anObject]!

WriteStream method!
pastEndPut: anObject
	"Grow the collection by creating a new bigger collection and then
	copy over the contents from the old one. We grow by doubling the size
	but the growth is kept between 20 and 1000000.
	Finally we put <anObject> at the current write position."

	| oldSize grownCollection |
	oldSize := collection size.
	grownCollection := collection class new: oldSize + ((oldSize max: 20) min: 1000000).
	collection := grownCollection replaceFrom: 1 to: oldSize with: collection startingAt: 1.
	writeLimit := collection size.
	collection at: (position := position + 1) put: anObject.

	^ anObject!

WriteStream method!
growTo: anInteger

   " anInteger is the required minimal new size of the collection "
	| oldSize grownCollection newSize |
	oldSize := collection size.
     newSize := anInteger + (oldSize // 4 max: 20).
	grownCollection := collection class new: newSize.
	collection := grownCollection replaceFrom: 1 to: oldSize with: collection startingAt: 1.
	writeLimit := collection size!


WriteStream method!
nextPutAll: aCollection
	| newEnd |

	newEnd := position + aCollection size.
	newEnd > writeLimit
		ifTrue: [self growTo: newEnd + 10].

	collection replaceFrom: position + 1 to: newEnd  with: aCollection startingAt: 1.
	position := newEnd!


WriteStream method!
cr
	self nextPut: Character cr!

WriteStream method!
tab
	self nextPut: Character tab!


WriteStream method!
contents
	readLimit := readLimit max: position.
	^ collection copyFrom: 1 to: position!


"private"

WriteStream method!
on: aCollection
	super on: aCollection.
	readLimit := 0.
	writeLimit := aCollection size!