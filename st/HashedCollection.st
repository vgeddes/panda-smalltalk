

HashedCollection classMethod!
new
	^ self new: self defaultCapacity!

HashedCollection classMethod!
new: initialCapacity
	^ self basicNew initialize: initialCapacity!

HashedCollection method!
at: index include: anObject
	| index |

	array at: index put: anObject. 
	size := size + 1.

	self growIfNeeded.

	^ anObject!

HashedCollection method!
removeAtIndex: index
	array at: index put: array.
	size := size - 1.
	deleted := deleted + 1!


HashedCollection method!
find: anObject
	^ self find: anObject in: array!

HashedCollection method!
find: anObject in: anArray
	self subclassResponsibility!

HashedCollection method!
grow
	| newArray i |

	newArray := Array new: (array size + array size).
	
	self do: [ :object |
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
	^ size!

HashedCollection method!
capacity
	^ array size // 2!


"enumerating"
HashedCollection method!
do: aBlock
	1 to: array size do:
		[ :i | | object |
			object := array at: i.
			(object ~~ nil) & (object ~~ array) ifTrue: [ aBlock value: object ]]!


"private"

HashedCollection classMethod!
minimumCapacity
	^ 4!

HashedCollection classMethod!
defaultCapacity
	^ 8!

HashedCollection method!
sizeForCapacity: capacity
	| minsize |

	minsize := HashedCollection minimumCapacity.

	[ minsize < capacity ]
		whileTrue: [ minsize := minsize + minsize ].

	^ minsize!

HashedCollection method!
occupiedCount
	^ size + deleted!

HashedCollection method!
initialize: initialCapacity
	array := Array new: (self sizeForCapacity: initialCapacity).
	size := 0.
	deleted := 0!

HashedCollection method!
loadFactor
	^ self occupiedCount / array size!
