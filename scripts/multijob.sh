#execute ./jobCommander issueJob ./file for every line of every file given in the terminal
for job_file in "$@"; do

    while read -r line; do
        ./jobCommander issueJob "$line" #inspration from https://stackoverflow.com/questions/10929453/read-a-file-line-by-line-assigning-the-value-to-a-variable
    done < "$job_file"
done