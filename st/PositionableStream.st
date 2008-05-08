

"instance creation"

PositionableStream classMethod!
on: aCollection
	^ self basicNew on: aCollection!
Annotation key: 'category' value: 'instance creation'!


"testing"

PositionableStream method!
isEmpty
	^ position = 0!
Annotation key: 'category' value: 'testing'!


"accessing"

PositionableStream method!
peek
	| nextObject |
	self atEnd ifTrue: [^ nil].
	nextObject := self next.
	position := position - 1.
	^ nextObject!
Annotation key: 'category' value: 'accessing'!

PositionableStream method!
contents
	^ collection copyFrom: 1 to: readLimit!
Annotation key: 'category' value: 'accessing'!

PositionableStream method!
last
	^ collection at: position!
Annotation key: 'category' value: 'accessing'!


"positioning"

PositionableStream method!
position
	^ position!
Annotation key: 'category' value: 'positioning'!

PositionableStream method!
position: anInteger
	anInteger >= 0 & (anInteger <= readLimit)
		ifTrue: [position := anInteger]
		ifFalse: [self positionError]!
Annotation key: 'category' value: 'positioning'!

PositionableStream method!
reset
	position := 0!
Annotation key: 'category' value: 'positioning'!

PositionableStream method!
setToEnd
	position := readLimit!
Annotation key: 'category' value: 'positioning'!


"private"

PositionableStream method!
on: aCollection
	collection := aCollection.
	readLimit := aCollection size.
	position := 0.
	self reset!