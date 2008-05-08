
"accessing"

Stream method!
next
	self subclassResponsibility!
Annotation key: 'category' value: 'accessing'!

Stream method!
contents
	self subclassResponsibility!
Annotation key: 'category' value: 'accessing'!

Stream method!
nextPut: anObject
	self subclassResponsibility!
Annotation key: 'category' value: 'accessing'!

Stream method!
nextPutAll: aCollection
	aCollection do: [ :element | self nextPut: element].
	^ aCollection!
Annotation key: 'category' value: 'accessing'!

Stream method!
next: anInteger put: anObject
	anInteger timesRepeat: [self nextPut: anObject].
	^ anObject!
Annotation key: 'category' value: 'accessing'!


"enumerating"
Stream method!
do: aBlock
	[self atEnd] whileFalse: [aBlock value: self next]!
Annotation key: 'category' value: 'enumerating'!


"testing"

Stream method!
atEnd
	self subclassResponsibility!
Annotation key: 'category' value: 'testing'!