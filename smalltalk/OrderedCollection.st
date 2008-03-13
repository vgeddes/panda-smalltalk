!OrderedCollection methodsFor: 'basic'!

add: anObject
    self growBy: 1.
    self at: self size put: anObject
!

addAll: aCollection
    aCollection do: [ :ea |
	self add: ea ]
!

remove: anObject ifAbsent: aBlock
    ^self removeAtIndex:
        (self indexOf: anObject startingAt: 1 ifAbsent: [ ^aBlock value ])
!

removeAtIndex: anIndex
    ^self notYetImplemented
! !
