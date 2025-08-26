result="./Tests/AdditionalFiles/file_for_result"

while IFS= read -r line; do
  grep_output=$(grep $line < ./Tests/Flags.txt 2>/dev/null)
  my_grep_output=$(./s21_grep $line < ./Tests/Flags.txt 2>/dev/null)

  if [[ "$grep_output" == "$my_grep_output" ]]; then
    echo "$line: Файлы идентичны"
  else
    echo "$line: Файлы различны"
    echo "----------------GREP-----------------"
    echo "$grep_output"
    echo "-------------------------------------"
    echo "---------------MY GREP--------------"
    echo "$my_grep_output"
    echo "-------------------------------------"
    diff <(echo "$grep_output") <(echo "$my_grep_output")
    echo "$line" >> "$result"
  fi
done < "./Tests/Flags.txt"