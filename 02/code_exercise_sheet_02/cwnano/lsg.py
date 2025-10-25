guess_char = ";"
current_max = 0

ref_trace = cap_pass_trace(passwort + ";")[50:200]
for c in tqdm('abcdefghij'):
    trace = cap_pass_trace(passwort + c)[50:200]
    
    plt.plot(np.abs(ref_trace - trace), label=c)
    plt.legend(ncol=2, bbox_to_anchor=[1.05,1], loc='upper left')
    plt.subplots_adjust(right=0.7)
    plt.show()
    
    if current_max < np.sum(np.abs(ref_trace - trace)):
        current_max = np.sum(np.abs(ref_trace - trace))
        guess_char = c
        print(f"{c}: {current_min}")

print(guess_char)



















int main(int param_anz, char** args) {
  printf("%s\n", args[3]);
  
  }
