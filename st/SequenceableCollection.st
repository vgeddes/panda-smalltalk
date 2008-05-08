

"accessing"

SequenceableCollection method!
size
	self subclassResponsibility!
Annotation key: 'category' value: 'accessing'!

SequenceableCollection method!
replaceFrom: start to: stop with: replacement startingAt: repStart 
	"This destructively replaces elements from start to stop in the receiver 
	starting at index, repStart, in the sequenceable collection, 
	replacementCollection. Answer the receiver. No range checks are 
	performed."

	| index repOff |
	repOff := repStart - start.
	index := start - 1.
	[(index := index + 1) <= stop]
		whileTrue: [self at: index put: (replacement at: repOff + index)]!
Annotation key: 'category' value: 'accessing'!


"removing "

SequenceableCollection method!
remove: anObject ifAbsent: anExceptionBlock
	self shouldNotImplement!
Annotation key: 'category' value: 'removing'!


"enumerating"

SequenceableCollection method!
do: aBlock
	| index length |
	index := 0.
	length := self size.
	[(index := index + 1) <= length]
		whileTrue: [ aBlock value: (self at: index) ].!
Annotation key: 'category' value: 'enumerating'!

SequenceableCollection method!
collect: aBlock
	| aStream index length |
	aStream := WriteStream on: (self species new: self size).
	index := 0.
	length := self size.
	[(index := index + 1) <= length]
		whileTrue: [aStream nextPut: (aBlock value: (self at: index))].
	^ aStream contents!
Annotation key: 'category' value: 'enumerating'!

SequenceableCollection method!
select: aBlock
	| aStream index length |
	aStream := WriteStream on: (self species new: self size).
	index := 0.
	length := self size.
	[(index := index + 1) <= length]
		whileTrue: [(aBlock value: (self at: index))
						ifTrue: [aStream nextPut: (self at: index)]]. 
	^ aStream contents!
Annotation key: 'category' value: 'enumerating'!


"copying"

SequenceableCollection method!
copyFrom: start to: stop 
	"Answer a copy of a subset of the receiver, starting from element at 
	index start until element at index stop."

	| newSize |
	newSize := stop - start + 1.
	^ (self species new: newSize)
		replaceFrom: 1
		to: newSize
		with: self
		startingAt: start!
Annotation key: 'category' value: 'copying'!