

"Link"

Link method!
value: anObject
	value := anObject!

Link method!
value
	^ value!

Link method!
next: aLink
	next := aLink!

Link method!
next
	^ next!

"List"	

List method!
add: anObject
	| link |

	link := Link new.
	link value: anObject.

	first == nil
		ifTrue: [first := link. last := link]
		ifFalse: [last next: link. last := link].
		
	size := size + 1.
	^ anObject!


List method!
addFirst: anObject
	| link |

	link := Link new.
	link value: anObject.

	first == nil
		ifTrue:
			[ first := link.
			  last := link ]
		ifFalse:
			[ link next: first.
			  first := link ].

	size := size + 1.

	^ anObject!


List method!
addLast: anObject
	^ self add: anObject!

List method!
do: aBlock
	| link |
	link := first.
	[ link ~~ nil ]
		whileTrue: [aBlock value: link value.
			        link := link next]!

List method!
select: aBlock
	| aStream link |
	aStream := WriteStream on: (self species new: self size).
	link := first.
	[link ~~ nil ]
		whileTrue: [(aBlock value: link value)
						ifTrue: [aStream nextPut: (link value)].
					link := link next]. 
	^ aStream contents!

List method!
collect: aBlock
	| aStream link |
	aStream := WriteStream on: (self species new: self size).
	link := first.
	[link ~~ nil]
		whileTrue: [aStream nextPut: (aBlock value: (link value)).
					link := link next].
	^ aStream contents!


List method!
findLink: index
	| i link |

	i := 1.
	link := first.
	[ (link ~= nil) & (i < index) ]
		whileTrue: [ i := i + 1. link := link next].

	self assert: [ link ~= nil ].
	^ link!

List method!
at: index
	| link |

	self emptyCheck.

	(index < 1) | (index > size)
		ifTrue: [self error: 'out of bounds' ]. 

	link := self findLink: index.
	^ link value!

List method!
at: index put: anObject
	| link |

	self emptyCheck.

	(index < 1) | (index > size)
		ifTrue: [self error: 'out of bounds' ]. 

	link := self findLink: index.
	link value: anObject.
	^ anObject!

List method!
size
	^ size!

List method!
initialize
	size := 0!

List method!
species
	^ Array!

List method!
first
	self emptyCheck.
	^ first value!

List method!
last
	self emptyCheck.
	^ last value!

List method!
removeFirst
	| removed |

	self emptyCheck.

	removed := first value.
	first == last
		ifTrue: [first := nil. last := nil]
		ifFalse: [first := first next].

	size := size - 1.
	^ removed!

List method!
removeLast
	| link removed |

	self emptyCheck.

	removed := last value.
	first == last
		ifTrue: [first := nil. last := nil]
		ifFalse: [link := first.		
				  [link next == last ]
					  whileFalse: [ link := link next].
				  link next: nil.
				  last := link].

	size := size - 1.
	^ removed!

List method!
remove: anObject ifAbsent: aBlock
	| link next |

	link := first.
	link ifNil: [^ aBlock value].

	link value = anObject
		ifTrue:  [first == last
					  ifTrue: [last := nil].
				  first := first next]
		ifFalse: [[link next ifNil: [^ aBlock value].
				   link next value ~= anObject]
		 			   whileTrue: [link := link next].
				   link next: link next next].

	size := size - 1.
	^  anObject!


List method!
emptyCheck
	first == nil ifTrue: [ self error: 'list is empty' ]!