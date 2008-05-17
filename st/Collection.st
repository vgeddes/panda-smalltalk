

"adding"

Collection method!
add: anObject
	self subclassResponsibility!

Collection method!
addAll: aCollection
	aCollection do: [ :each | self add: each ].
	^ aCollection! 


"removing"

Collection method!
remove: anObject ifAbsent: anExceptionBlock
	self subclassResponsibility!

Collection method!
remove: anObject
	self remove: anObject ifAbsent: [self errorNotFound]!

Collection method!
removeAll: aCollection
	aCollection do: [ :each | self remove: each ].
	^ aCollection! 


"testing"

Collection method!
isEmpty
	^ self size = 0!

Collection method!
includes: anObject
	self do: [ :each | anObject = each ifTrue: [^ true]].
	^ false!

Collection method!
occurancesOf: anObject
	| tally |
	tally := 0.
	self do: [ :each | anObject = each ifTrue: [ tally := tally + 1]].
	^ tally!


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

Collection method!
collect: aBlock
	| newCollection |
	newCollection := self species new.
	self do: [ :each | newCollection add: (aBlock value: each)].
	^ newCollection!

Collection method!
reject: aBlock
	^ self select: [ :element | (aBlock value: element) == false]!

Collection method!
select: aBlock
	| newCollection |
	newCollection := self species new.
	self do: [ :each | (aBlock value: each) ifTrue: [newCollection add: each ]].
	^ newCollection!

"private"

Collection method!
errorNotFound
	self error: 'Object is not in collection'!

Collection method!
species
	^ self class!