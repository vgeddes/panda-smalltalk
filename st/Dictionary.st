
"accessing"

Dictionary method!
at: key
	| assoc |

	assoc := array at: (self find: key)
	assoc ifNil: [^ nil].

	^ assoc value!

Dictionary method!
at: key put: anObject
	| index assoc |

	index := self find: key.
	assoc := array at: index.

	assoc ifNotNil: [assoc value: anObject. ^ anObject].

	self at: index include: (Association key: key value: anObject).

	^ anObject!

Dictionary method!
keys
	| keys |
	keys := Set new.
	self do: [ :assoc | keys include: assoc key ].
	^ keys!

"testing"
Dictionary method!
includesKey: anObject
	| index assoc |

	(array at: (self find: anObject)) ifNil: [^ false].

	^ true!

"private"

Dictionary method!
remove: anObject ifAbsent: aBlock
	self shouldNotImplement!

Dictionary method!
removeKey: anObject ifAbsent: aBlock
	| index assoc |

	index := self find: anObject.
	assoc := array at: index.
	assoc ifNil: [^ aBlock value]. 

	self removeAtIndex: index.

	^ assoc value!

Dictionary method!
removeKey: anObject
	^ self removeKey: anObject ifAbsent: [self error: 'key not found']!

Dictionary method!
find: anObject in: anArray
	| i mask |

	mask := anArray size - 1.

	i := (anObject hash bitAnd: mask) + 1.

	[ | object | 

	  object := anArray at: i.

	  (object == nil)
		  ifTrue: [^ i].
	  (object key = anObject) 
		  ifTrue: [^ i].

	  i := (i + 106720 bitAnd: mask) + 1.

	] repeat!