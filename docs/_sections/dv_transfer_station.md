---
layout: post
title: DV Transfer Station
---

# Equipment Lists & Set-up




# Decks (Overview)

You can see the DV equipment list spreadsheet [here](https://docs.google.com/spreadsheets/d/1HvNsC110aBH8WjfnNJrF6KlT2STgCT5KRBaf1g6MxGQ/edit#gid=0) while we work on it. Below is the contents of this spreadsheet, minus the images.

{% for row in site.data.decks %}

## {{ row["Make"] }} {{ row["Model"] }}

**Standard(s)**: {{ row["Standard"] }}  
**Compatible Formats**: {{ row["Compatible Formats"] }}  
**Compatible Sizes**: {{ row["Compatible Sizes"] }}  
**Non-compatible Formats**: {{ row["Non-compatible Formats"] }}  
{% if row["Cable/Input"] %}
**Cable/Input**: {{ row["Cable/Input"] }}  
{% endif %}

{% if row["Known Issues/Unique Characteristics"] %}
**Known Issues/Unique Characteristics**: {{ row["Known Issues/Unique Characteristics"] }}
{% endif %}

**Manuals**  
{% unless row["Operation Manual"] %}{% unless row["Service Manual"] %}None available{% endunless %}{% endunless %}{% if row["Operation Manual"] %}<a href='{{ row["Operation Manual"] }}'>Operation Manual</a>{% endif %}  
{% if row["Service Manual"] %}<a href='{{ row["Service Manual"] }}'>Service Manual</a>{% endif %}

{% endfor %}


# Maintenance & Cleaning

This section still needs to be done!
