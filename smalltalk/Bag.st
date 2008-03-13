!Bag methodsFor: 'accessing'!

indexOf: anObject
    "This method is similiar to Set>>indexOf: except that return the position only of empty entries"
    | h2 pos size |
    size := self basicSize.
    pos := anObject hash \\ (size - 1).
    h2 := pos quo: 4.

    h2 := h2 max: 1.
    (h2 ~= 1 & ((h2 \\ 2) isZero))
	ifTrue: [ h2 := h2 - 1 ].
    
    pos := pos + 1.
    1 to: size do: [ :i |
	| key |
	key := self basicAt: pos.
	key isNil
	    ifTrue: [ ^pos ].
	pos := pos + h2.
	pos > size
	    ifTrue: [ pos := pos - size ] ].
    self error: 'Full'
! !
