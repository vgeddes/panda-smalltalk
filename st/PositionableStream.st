

"instance creation"


PositionableStream classMethod!
on: aCollection
	^ self basicNew on: aCollection!


"testing"

PositionableStream method!
isEmpty
	^ position = 0!

"accessing"

PositionableStream method!
peek
	| nextObject |
	self atEnd ifTrue: [^ nil].
	nextObject := self next.
	position := position - 1.
	^ nextObject!

PositionableStream method!
contents
	^ collection copyFrom: 1 to: readLimit!

PositionableStream method!
last
	^ collection at: position!


"positioning"

PositionableStream method!
position
	^ position!

PositionableStream method!
position: anInteger
	anInteger >= 0 & (anInteger <= readLimit)
		ifTrue: [position := anInteger]
		ifFalse: [self positionError]!

PositionableStream method!
reset
	position := 0!

PositionableStream method!
setToEnd
	position := readLimit!



"private"

PositionableStream method!
on: aCollection
	collection := aCollection.
	readLimit := aCollection size.
	position := 0.
	self reset!
