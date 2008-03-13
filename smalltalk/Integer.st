!Integer methodsFor: 'bit-wise operations'!

bitAnd: aNumber
    aNumber isInteger
	ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
	ifTrue: [ ^self bitAnd: (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) bitAnd: aNumber ]
!

bitOr: aNumber
    aNumber isInteger
	ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
	ifTrue: [ ^self bitOr: (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) bitOr: aNumber ]
!

bitXor: aNumber
    aNumber isInteger
	ifFalse: [ self error: 'An integer is required' ].

    self generality > aNumber generality
	ifTrue: [ ^self bitXor: (self coerce: aNumber) ]
	ifFalse: [ ^(aNumber coerce: self) bitXor: aNumber ]
!

bitShift: aNumber
    aNumber isSmallInteger
	ifFalse: [ self error: 'A small integer is required' ].
   
    ^self asLargeInteger bitShift: aNumber
!

<< aNumber
    ^self bitShift: aNumber
!

>> aNumber
    ^self bitShift: aNumber negated
! !

!Integer methodsFor: 'converting'!

truncated
    ^self
! !

!Integer methodsFor: 'testing'!

isLargeInteger
    ^self subclassResponsibility
!

isSmallInteger
    ^self subclassResponsibility
!

isInteger
    ^true
!

isFloat
    ^false
! !
