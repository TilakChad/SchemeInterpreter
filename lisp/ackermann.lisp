(define (ackermann x y)
		(cond  ((= y 0) 0)
			   ((= x 0) (* 2 y))
			   ((= y 1) 2)
			   (else 
					(ackermann (- x 1) (ackermann x (- y 1))))
		)
)

(ackermann 2 4)

(ackermann 0 (ackermann 1 3))

// SICP -> Exercise  1.10 

(A 1 10) ->
(A 2 4)  -> 
(A 3 3)  -> 

(define (f n) (ackermann 0 n)) -> 2n
(define (g n) (ackermann 1 n)) -> 2 *  ackermann(0,ackermann(1,n-1)) -> 2^n  
(define (h n) (ackermann 2 n)) -> ackermann(1,ackermann(2,n-1)) -> 2^(ackermann(2,n-1)) ->  2 ^ (2 ^ ackermann(2,0)) -> 2 ^ (2 ^ (2  ^ (.. ^ 2) ) -> n times  
					(h 3) = 2^(2^2)  ->  16
					(h 4) = 2 ^ 16 -> (16384 * 4)