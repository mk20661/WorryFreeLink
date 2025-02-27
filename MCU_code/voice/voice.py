from gtts import gTTS

#text = f"You’ve been studying for 1 hour straight. Take a 5-10 minute break to boost your efficiency!"
#text = f"Select Eating"
#text = f"Select Exercising"
#text = f"Select Sleeping"
#text = f"Select social activities"
text = f"Select other activities"


tts = gTTS(text, lang='en')
#tts.save("./eating.mp3")
#tts.save("./exercising.mp3")
#tts.save("./sleeping.mp3")
#tts.save("./social.mp3")
tts.save("./others.mp3")