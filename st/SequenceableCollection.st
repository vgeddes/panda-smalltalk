

"copying"

SequenceableCollection method!
, aCollection
	| collection |

	collection := self species new: (self size + aCollection size).

	collection replaceFrom: 1
			   to: self size
			   with: self
			   startingAt: 1.

	collection replaceFrom: self size + 1
			   to: collection size
			   with: aCollection
			   startingAt: 1.

	^ collection!


"comparing"

SequenceableCollection method!
= aCollection
	| i |
	
	self size = aCollection size
		ifFalse: [ ^ false ].

	i := 1.
	self do: [ :element |
		element = (aCollection at: i) ifFalse: [ ^ false ].
		i := i + 1 ].

	^ true!

SequenceableCollection method!
hash
	| hash |

	hash := 11111111111111111.

	self do: [ :element |
		hash := hash bitXor: ((hash bitShift: 5) + element hash + (hash bitShift: -2))].

	^ hash!

"finding"


"accessing"
SequenceableCollection method!
findFirst: aBlock
	| i el |
	i := 0.
	[(i := i + 1) <= self size]
		whileTrue:
			[ el := self at: i.
			 (aBlock value: el) ifTrue: [^ el]].

	 ^ nil!

"accessing"
SequenceableCollection method!
findLast: aBlock
	| i el |
	i := self size + 1.
	[(i := i - 1) >= 1]
		whileTrue:
			[ el := self at: i.
			 (aBlock value: el) ifTrue: [^ el]].

	 ^ nil!

SequenceableCollection method!
size
	<primitive: 'SequenceableCollection_size'>
	self primitiveFailed!


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


SequenceableCollection method!
atAllPut: anObject
	| index size |

	index := 0.
	size := self size.

	[ (index := index + 1) <= size ]
		whileTrue: [ self at: index put: anObject ]!


SequenceableCollection method!
first
	^ self at: 1!

SequenceableCollection method!
second
	^ self at: 2!

SequenceableCollection method!
third
	^ self at: 3!

SequenceableCollection method!
last
	^ self at: self size!


"removing "

SequenceableCollection method!
remove: anObject ifAbsent: anExceptionBlock
	self shouldNotImplement!


"enumerating"

SequenceableCollection method!
do: aBlock
	| index length |
	index := 0.
	length := self size.
	[(index := index + 1) <= length]
		whileTrue: [ aBlock value: (self at: index) ].!

SequenceableCollection method!
collect: aBlock
	| aStream index length |
	aStream := WriteStream on: (self species new: self size).
	index := 0.
	length := self size.
	[(index := index + 1) <= length]
		whileTrue: [aStream nextPut: (aBlock value: (self at: index))].
	^ aStream contents!

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


"converting"
SequenceableCollection method!
reversed
	"Answer a copy of the receiver with element order reversed."
	"Example: 'frog' reversed"

	| n result src |
	n := self size.
	result := self species new: n.
	src := n + 1.
	1 to: n do: [:i | result at: i put: (self at: (src := src - 1))].
	^ result!