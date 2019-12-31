<?xml version="1.0"?>
<!-- WIP -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:dv="https://mediaarea.net/dvrescue" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:x="http://www.w3.org/1999/xhtml" version="1.0" extension-element-prefixes="xsi" exclude-result-prefixes="dv x" >
<xsl:output encoding="UTF-8" method="html" version="1.0" indent="yes" doctype-system="about:legacy-compat"/>
<xsl:template match="/dv:dvrescue">
  <html>
    <head>
      <style type="text/css">
        body {
          font-family: "PT Sans", Helvetica, Arial, sans-serif;
          max-width: 1500px;
          display: grid;
          justify-content: center;
          justify-items: center;
          overflow-wrap: anywhere;
        }
        h1 {
          font-weight: 800;
          font-style: italic;
        }
        img {
          width: 300px;
          height: inherit;
        }
        td {
          padding-left: 2em;
          padding-right: 2em;
        }
        dt {
          font-weight: 800;
          font-style: italic;
        }
        .frames {
          display: flex;
          flex-wrap: wrap;
          justify-content: center;
        }
        .frameError {
          border: 2px solid red;
          max-width: 300px;
        }
        .frameOK {
          border: 2px solid black;
          width: 300px;
        }
        .frameOK p {
          padding: 3.33em;
        }
      </style>
    </head>
    <body>
      <xsl:for-each select="dv:media">
        <header>
          <h1>DVRescue Report</h1>
          <h2><xsl:value-of select="@ref"/></h2>
        </header>
        <xsl:for-each select="dv:frames">
          <section class="metadata">
            <!-- These need to only appear if present -->
            <table>
              <thead><tr><th colspan="2">Frames metadata</th></tr></thead>
              <tbody>
                <tr>
                  <td>
                    <dl>
                      <dt>Count</dt><dd><xsl:value-of select="@count"/></dd>
                      <dt>Start timestamp</dt><dd><xsl:value-of select="@pts"/></dd>
                      <dt>End timestamp</dt><dd><xsl:value-of select="@end_pts"/></dd>
                      <dt>Size</dt><dd><xsl:value-of select="@size"/></dd>
                      <dt>Video rate</dt><dd><xsl:value-of select="@video_rate"/></dd>
                    </dl>
                  </td>
                  <td>
                    <dl>
                      <dt>Aspect ratio</dt><dd><xsl:value-of select="@aspect_ratio"/></dd>
                      <dt>Chroma subsampling</dt><dd><xsl:value-of select="@chroma_subsampling"/></dd>
                      <dt>Audio rate</dt><dd><xsl:value-of select="@audio_rate"/></dd>
                      <dt>Channels</dt><dd><xsl:value-of select="@channels"/></dd>
                    </dl>
                  </td>
                </tr>
              </tbody>
            </table>
          </section>
          <section class="frames">
            <xsl:for-each select="dv:frame">
              <xsl:choose>
                <xsl:when test="dv:dseq/dv:sta">
                  <div class="frameError">
                    <img>
                      <xsl:attribute name="src"><xsl:value-of select="concat(../../@ref,'_dvrescue/', translate(@tc,':','-'))"/>.jpg</xsl:attribute>
                    </img>
                    <p class="tc"><xsl:value-of select="@tc"/></p>
                    <p>Arbitrary data: <xsl:value-of select="@arb"/></p>
                    <xsl:if test="@arb_r"><p>Arbitrary data is repeating.</p></xsl:if>
                    <xsl:if test="@arb_nc"><p>Arbitrary data is non-consecutive.</p></xsl:if>
                    <p>Error #<xsl:value-of select="dv:dseq/dv:sta/@t"/></p>
                    <xsl:if test="dv:dseq/dv:sta/@t=0"><p>No error, what a nice DV macroblock.</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=2"><p>Replaced a macroblock with the one of the same position of the previous frame (guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=4"><p>Replaced a macroblock with the one of the same position of the next frame (guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=6"><p>A concealment method is used but not specified (guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=7"><p>Error with an error code within the macro block.</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=10"><p>Replaced a macroblock with the one of the same position of the previous frame (no guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=12"><p>Replaced a macroblock with the one of the same position of the next frame (no guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=14"><p>A concealment method is used but not specified (no guaranteed continuity).</p></xsl:if>
                    <xsl:if test="dv:dseq/dv:sta/@t=15"><p>Error with unknown position.</p></xsl:if>
                  </div>
                </xsl:when>
              </xsl:choose>
            </xsl:for-each>
          </section>
        </xsl:for-each>
      </xsl:for-each>
      <footer><xsl:value-of select="dv:creator/dv:program"/> v.<xsl:value-of select="dv:creator/dv:version"/></footer>
    </body>
  </html>
  </xsl:template>
</xsl:stylesheet>
