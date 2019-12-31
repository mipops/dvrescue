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
          max-width: 400px;
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
          border: 0.2em solid black;
          margin: 0.1em;
          max-width: 400px;
        }
        .frameError p {
          padding-left: 0.5em;
          padding-right: 0.5em;
          margin: 0.5em;
        }
        .tc {
          background-color: black;
          color: white;
          font-weight: bold;
          font-size: 1.5em;
          text-align: center;
          padding: 0;
          margin: 0 !important;
        }
        .errorNum {
          text-decoration: underline wavy;
        }
        .errorNum:hover .tooltip {
          display: block;
        }
        .tooltip {
          display: none;
          background-color: black;
          color: white;
          font-weight: bold;
          margin-left: 1em;
          border-radius: 5px;
          padding: 0.3em;
          position: absolute;
          z-index: 1000;
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
            <table>
              <thead><tr><th colspan="2">Frames metadata</th></tr></thead>
              <tbody>
                <tr>
                  <td>
                    <dl>
                      <xsl:if test="@count"><dt>Count</dt><dd><xsl:value-of select="@count"/></dd></xsl:if>
                      <xsl:if test="@scan_type"><dt>Scan type</dt><dd><xsl:value-of select="@scan_type"/></dd></xsl:if>
                      <xsl:if test="@pts"><dt>Start timestamp</dt><dd><xsl:value-of select="@pts"/></dd></xsl:if>
                      <xsl:if test="@end_pts"><dt>End timestamp</dt><dd><xsl:value-of select="@end_pts"/></dd></xsl:if>
                      <xsl:if test="@size"><dt>Size</dt><dd><xsl:value-of select="@size"/></dd></xsl:if>
                      <xsl:if test="@video_rate"><dt>Video rate</dt><dd><xsl:value-of select="@video_rate"/></dd></xsl:if>
                    </dl>
                  </td>
                  <td>
                    <dl>
                      <xsl:if test="@aspect_ratio"><dt>Aspect ratio</dt><dd><xsl:value-of select="@aspect_ratio"/></dd></xsl:if>
                      <xsl:if test="@chroma_subsampling"><dt>Chroma subsampling</dt><dd><xsl:value-of select="@chroma_subsampling"/></dd></xsl:if>
                      <xsl:if test="@audio_rate"><dt>Audio rate</dt><dd><xsl:value-of select="@audio_rate"/></dd></xsl:if>
                      <xsl:if test="@channels"><dt>Channels</dt><dd><xsl:value-of select="@channels"/></dd></xsl:if>
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
                      <xsl:attribute name="src"><xsl:value-of select="translate(@tc,':','-')"/>.jpg</xsl:attribute>
                    </img>
                    <xsl:if test="@tc"><p class="tc"><xsl:value-of select="@tc"/></p></xsl:if>
                    <xsl:if test="@n"><p class="tc"><xsl:value-of select="@n"/></p></xsl:if>
                    <xsl:if test="@rec_start"><p>Recording start</p></xsl:if>
                    <xsl:if test="@rec_end"><p>Recording end</p></xsl:if>
                    <xsl:if test="@rdt">
                      <p>Recorded Date Time 
                      <xsl:value-of select="@rdt"/>
                      <xsl:if test="@rdt_r"> (repeating) </xsl:if>
                      <xsl:if test="@rdt_nc"> (non-consecutive) </xsl:if>
                      </p>
                    </xsl:if>
                    <xsl:if test="@arb">
                      <p>Arbitrary data: <xsl:value-of select="@arb"/>
                      <xsl:if test="@arb_r"> (repeating)</xsl:if>
                      <xsl:if test="@arb_nc"> (non-consecutive)</xsl:if>
                      </p>
                    </xsl:if>
                    <xsl:for-each select="dv:dseq/dv:sta">
                      <p>DIF sequence number: <xsl:value-of select="../@n"/></p>
                      <xsl:call-template name="staType"/>
                    </xsl:for-each>
                    <xsl:for-each select="dv:dseq/dv:aud">
                      <xsl:call-template name="audType"/>
                    </xsl:for-each>
                    <xsl:for-each select="dv:sta">
                      <xsl:call-template name="staType"/>
                    </xsl:for-each>
                    <xsl:for-each select="dv:aud">
                      <xsl:call-template name="audType"/>
                    </xsl:for-each>
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
  <xsl:template match="dv:sta" name="staType">
    <xsl:if test="@t">
      <p class="errorNum">Error #<xsl:value-of select="@t"/>
        <span class="tooltip">
          <xsl:if test="@t=0">No error, what a nice DV macroblock.</xsl:if>
          <xsl:if test="@t=2">Replaced a macroblock with the one of the same position of the previous frame (guaranteed continuity).</xsl:if>
          <xsl:if test="@t=4">Replaced a macroblock with the one of the same position of the next frame (guaranteed continuity).</xsl:if>
          <xsl:if test="@t=6">A concealment method is used but not specified (guaranteed continuity).</xsl:if>
          <xsl:if test="@t=7">Error with an error code within the macro block.</xsl:if>
          <xsl:if test="@t=10">Replaced a macroblock with the one of the same position of the previous frame (no guaranteed continuity).</xsl:if>
          <xsl:if test="@t=12">Replaced a macroblock with the one of the same position of the next frame (no guaranteed continuity).</xsl:if>
          <xsl:if test="@t=14">A concealment method is used but not specified (no guaranteed continuity).</xsl:if>
          <xsl:if test="@t=15">Error with unknown position.</xsl:if>
        </span>
      </p>
    </xsl:if>
    <p><strong>[ STA </strong>
    <xsl:if test="@n"><strong>Count </strong> <xsl:value-of select="@n"/></xsl:if>
    <xsl:if test="@n_even"><strong> | Even </strong> <xsl:value-of select="@n_even"/><strong> | Odd </strong> <xsl:value-of select="number(@n) - number(@n_even)"/>
    </xsl:if><strong> ]</strong></p>
  </xsl:template>
  <xsl:template match="dv:aud" name="audType">
        <p><strong>[ AUD </strong>
    <xsl:if test="@n"><strong>Count </strong> <xsl:value-of select="@n"/></xsl:if>
    <xsl:if test="@n_even"><strong> | Even </strong> <xsl:value-of select="@n_even"/><strong> | Odd </strong> <xsl:value-of select="number(@n) - number(@n_even)"/>
    </xsl:if><strong> ]</strong></p>
  </xsl:template>
</xsl:stylesheet>
