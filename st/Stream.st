
"accessing"

Stream method!
next
	self subclassResponsibility!

Stream method!
contents
	self subclassResponsibility!

Stream method!
nextPut: anObject
	self subclassResponsibility!

Stream method!
nextPutAll: aCollection
	aCollection do: [ :element | self nextPut: element].
	^ aCollection!

Stream method!
next: anInteger put: anObject
	anInteger timesRepeat: [self nextPut: anObject].
	^ anObject!


"printing"

Stream method!
print: anObject
	anObject printOn: self!


"enumerating"
Stream method!
do: aBlock
	[self atEnd] whileFalse: [aBlock value: self next]!


"testing"

Stream method!
atEnd
	self subclassResponsibility!


"print stream"

PrintStream classMethod!
new
	^ self on: (String new: 50)!

PrintStream method!
print: anObject
	self nextPutAll: anObject printString!

PrintStream method!
printString
	^ self contents!

PrintStream method!
cr
	self nextPut: Character cr!