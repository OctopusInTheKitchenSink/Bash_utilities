result="./Tests/AdditionalFiles/file_for_result"

while IFS= read -r line; do
  cat_output=$(cat $line < ./Tests/Flags.txt 2>/dev/null)
  my_cat_output=$(./s21_cat $line < ./Tests/Flags.txt 2>/dev/null)

  if [[ "$cat_output" == "$my_cat_output" ]]; then
    echo "$line: Файлы идентичны"
  else
    echo "$line: Файлы различны"
    echo "----------------CAT-----------------"
    echo "$cat_output"
    echo "-------------------------------------"
    echo "---------------MY CAT--------------"
    echo "$my_cat_output"
    echo "-------------------------------------"
    diff <(echo "$cat_output") <(echo "$my_cat_output")
    echo "$line" >> "$result"
  fi
done < "./Tests/Flags.txt"