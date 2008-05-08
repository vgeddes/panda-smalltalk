

"adding"

Collection method!
add: anObject
	self subclassResponsibility!
Annotation key: 'category' value: 'adding'!

Collection method!
addAll: aCollection
	aCollection do: [ :each | self add: each ].
	^ aCollection! 
Annotation key: 'category' value: 'adding'!


"removing"

Collection method!
remove: anObject ifAbsent: anExceptionBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'removing'!

Collection method!
remove: anObject
	self remove: anObject ifAbsent: [self errorNotFound]!
Annotation key: 'category' value: 'removing'!

Collection method!
removeAll: aCollection
	aCollection do: [ :each | self remove: each ].
	^ aCollection! 
Annotation key: 'category' value: 'removing'!


"testing"

Collection method!
isEmpty
	^ self size = 0!
Annotation key: 'category' value: 'testing'!

Collection method!
includes: anObject
	self do: [ :each | anObject = each ifTrue: [^ true]].
	^ false!
Annotation key: 'category' value: 'testing'!

Collection method!
occurancesOf: anObject
	| tally |
	tally := 0.
	self do: [ :each | anObject = each ifTrue: [ tally := tally + 1]].
	^ tally!
Annotation key: 'category' value: 'testing'!


"accessing"

Collection method!
size
	| tally |
	tally := 0.
	self do: [ :each | tally := tally + 1].
	^ tally!


"enumerating"

Collection method!
do: aBlock
	self subclassResponsibility!
Annotation key: 'category' value: 'enumerating'!

Collection method!
collect: aBlock
	| newCollection |
	newCollection := self species new.
	self do: [ :each | newCollection add: (aBlock value: each)].
	^ newCollection!
Annotation key: 'category' value: 'enumerating'!

Collection method!
reject: aBlock
	^ self select: [ :element | (aBlock value: element) == false]!
Annotation key: 'category' value: 'enumerating'!

Collection method!
select: aBlock
	| newCollection |
	newCollection := self species new.
	self do: [ :each | (aBlock value: each) ifTrue: [newCollection add: each ]].
	^ newCollection! 
Annotation key: 'category' value: 'enumerating'!


"private"

Collection method!
errorNotFound
	self error: 'Object is not in collection'!
Annotation key: 'category' value: 'private'!

Collection method!
species
	^ self class!
Annotation key: 'category' value: 'private'!