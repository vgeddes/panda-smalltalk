!Dictionary methodsFor: 'accessing'!

indexOf: aKey
    | h2 pos size |
    size := self basicSize.
    pos := 2 * (aKey hash \\ ((size - 1) quo: 2)).
    h2 := pos quo: 4.

    (h2 between: 0 and: 1)
	ifTrue: [ h2 := 2 ].
    (h2 \\ 2) isZero
	ifFalse: [ h2 := h2 - 1 ].

    1 to: size do: [ :i |
	| key |
	key := self basicAt: pos + 1.
	key == aKey | key isNil
	    ifTrue: [ ^pos + 1 ].
	pos := pos + h2.
	pos >= (size - 1)
	    ifTrue: [ pos := pos - (size - 1) ] ].

    self error: 'Dictionary is full'
! !
