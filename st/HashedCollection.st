

HashedCollection classMethod!
new
	^ self new: self defaultCapacity!

HashedCollection classMethod!
new: initialCapacity
	^ self basicNew initialize: initialCapacity!

HashedCollection method!
includes: anObject
	| index |

	index := self find: anObject.
	(array at: index) ifNil: [^ false].

	^ true!

HashedCollection method!
add: anObject
	| index |

	anObject == self
		ifTrue: [ self error: 'cannot insert self into dictionary' ].

	index := self find: anObject.
	(array at: index) ifNotNil: [ ^ anObject ].

	array at: index put: anObject. 
	tally := tally + 1.

	self growIfNeeded.

	^ anObject!

HashedCollection method!
remove: anObject
	^ self remove: anObject ifAbsent: []!

HashedCollection method!
remove: anObject ifAbsent: aBlock
	| index |

	anObject == self
		ifTrue: [ self error: 'cannot remove self from dictionary' ].

	index := self find: anObject.

	(array at: index) ifNil: [^ aBlock value]. 

	array at: index put: self.
	tally := tally - 1.
	deleted := deleted + 1.

	^ anObject!

HashedCollection method!
find: anObject
	^ self find: anObject in: array!

HashedCollection method!
find: anObject in: anArray
	| i mask |

	mask := anArray size - 1.

	i := ((self keyFor: anObject) hash bitAnd: mask) + 1.

	[ |  object | 

	  object := anArray at: i.

	  (object = (self valueFor: anObject)) | (object == nil)
		  ifTrue: [ ^ i].

	  i := (i + 106720 bitAnd: mask) + 1.

	] repeat!

HashedCollection method!
grow
	| newArray i |

	newArray := Array new: (array size + array size).
	
	self contents do: [ :object |
		newArray at: (self find: object in: newArray) put: object].

	array := newArray.
	deleted := 0!


HashedCollection method!
growIfNeeded
	"maximum 50% load factor"
	(self occupiedCount * 2) > array size
		ifTrue: [ self grow ]!


HashedCollection method!
size
	^ tally!

HashedCollection method!
capacity
	^ array size // 2!


"private"

HashedCollection classMethod!
minimumCapacity
	^ 4!

HashedCollection classMethod!
defaultCapacity
	^ 8!

HashedCollection method!
keyFor: anObject
	^ anObject!

HashedCollection method!
valueFor: anObject
	^ anObject!

HashedCollection method!
arraySizeForCapacity: capacity
	| size |

	size := HashedCollection minimumCapacity.

	[ size < capacity ]
		whileTrue: [ size := size + size ].

	^ size!

HashedCollection method!
occupiedCount
	^ tally + deleted!

HashedCollection method!
initialize: initialCapacity
	array := Array new: (self arraySizeForCapacity: initialCapacity).
	tally := 0.
	deleted := 0!

HashedCollection method!
loadFactor
	^ self occupiedCount / array size!

HashedCollection method!
contents
	^ array select: [ :object | (object ~~ nil) & (object ~~ self) ]!
