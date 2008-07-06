
IdentitySet method!
find: anObject in: anArray
	| i mask |

	mask := anArray size - 1.

	i := (anObject hash bitAnd: mask) + 1.

	[ | object | 

	  object := anArray at: i.

	  (object == anObject) | (object == nil)
		  ifTrue: [^ i].

	  i := (i + 106720 bitAnd: mask) + 1.

	] repeat!