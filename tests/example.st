fibonacci
   "Calculates the nth fibonacci number"
   (self > 1)
        ifTrue: [ ^ (self-2) fibonacci + (self-1) fibonacci ]
        ifFalse: [ ^ 1].
