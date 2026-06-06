program RangeOk;
type
  Digit == 0..5;
  Upper == 'A'..'Z';
  Color == (red, green, blue);
  Warm == red..green;
var
  i: Digit;
  c: Upper;
  w: Warm;
  letters: array['A'..'C'] of integer;
begin
  i := 4;
  c := 'Z';
  w := green;
  letters['A'] := i + 1;
  writeln(letters['A']);
end.
